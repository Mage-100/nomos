#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <array>
#include <utility>
#include <filesystem>
#include <ranges>
#include "MPSParser.hpp"
#include <string_view>

#include <Eigen/Sparse>

#include <print>

MPSParser::MPSParser(const std::filesystem::path& path) 
{
	std::ifstream file(path);
	
	// Checking if exists
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + path.string());
	}

	// load full file content on to content
	std::string content(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{});
	file.close();
	
	// create string viewing pointer 
	std::string_view text = content;

	while (!text.empty()) {
		const auto newlinePosition = text.find('\n');

		// terinary operations handling if end of file
		std::string_view line = text.substr(0, newlinePosition == std::string_view::npos ? text.size() : newlinePosition);

		// for Windows convert binarary -> str can add \r\n here so removing it 
		if (!line.empty() && line.back() == '\r') {
			line.remove_suffix(1);
		}
		_processLine(line);

		if (newlinePosition == std::string_view::npos) {
			break;
		}
		text.remove_prefix(newlinePosition + 1);
	}

	normalizeConstraintSigns();
	buildSlacks();
	buildArtificials();
	sortEntries();
	buildEigenSparseVectors();
}

bool MPSParser::_isSection(std::string_view line)
{
	// MPS section headers are left-aligned; data records are indented.  In
	// particular, an RHS record normally starts with the set name "RHS", so
	// recognizing it by its first token alone discards every RHS value.
	if (line.empty() || line.front() == ' ' || line.front() == '\t') {
		return false;
	}

	const std::string_view firstWord = _firstWord(line);

	// Checking whether the first word is a valid MPS section.
	return firstWord == "NAME"    ||
		   firstWord == "ROWS"    ||
		   firstWord == "COLUMNS" ||
		   firstWord == "RHS"     ||
		   firstWord == "BOUNDS"  ||
		   firstWord == "ENDATA";
}


MPSParser::MPSDATA_LINE MPSParser::_getSection(std::string_view line) 
{
	using DataLine = MPSParser::MPSDATA_LINE;

	const std::string_view sectionName = _firstWord(line);

	// Returning the corresponding MPS Section
	if (sectionName == "NAME") {
		return DataLine::NAME;
	}
	if (sectionName == "ROWS") {
		return DataLine::ROWS;
	}
	if (sectionName == "COLUMNS") {
		return DataLine::COLUMNS;
	}
	if (sectionName == "RHS") {
		return DataLine::RHS;
	}
	if (sectionName == "BOUNDS") {
		return DataLine::BOUNDS;
	}
	if (sectionName == "ENDATA") {
		return DataLine::ENDATA;
	}

	// Invalid section
	return DataLine::NAME;

}


std::pair<MPSParser::CONSTRAINT_TYPE, std::string>
	MPSParser::_catchConstraint(std::string_view line)
{
	using ConstraintType = MPSParser::CONSTRAINT_TYPE;

	std::istringstream inputStream{std::string(line)};

	std::string constraintTypeString;
	std::string rowName;

	ConstraintType  constraintType = ConstraintType::FREE;

	// Read the constraint type and row name from the line
	inputStream >> constraintTypeString >> rowName;

	// Converting the constraintTypeCode to corresponding enum values
	if (!constraintTypeString.empty()) {

		switch (constraintTypeString[0]) {
		case 'E':
			constraintType = ConstraintType::EQUAL;
			_equalityConstraints++;
			break;
		case 'L':
			constraintType = ConstraintType::LESS;
			_inequalityConstraints++;
			break;
		case 'G':
			constraintType = ConstraintType::GREATER;
			_inequalityConstraints++;
			break;
		case 'N':
			constraintType = ConstraintType::FREE;
			break;
		}
	}

	// The objective (N) row is not a constraint.  Constraint rows must be
	// numbered densely because they become the rows of A and b.
	if (constraintType == ConstraintType::FREE) {
		rowMap[rowName] = 0;
		return { constraintType, rowName };
	}

	rowMap[rowName] = _rowIndex;
	constraintMapRaw.insert({ _rowIndex, constraintType });
	_rowIndex++;

	return { constraintType, rowName };
}

// Builds the Slacks vector
void MPSParser::buildSlacks() {
	int slackCol = _columnIndex;
	for (int r = 0; r < getRowCount(); r++) {
		auto type = constraintMapRaw[r];
		if (type == CONSTRAINT_TYPE::FREE || type == CONSTRAINT_TYPE::EQUAL) continue;
		double value = (type == CONSTRAINT_TYPE::LESS) ? 1.0 : -1.0;
		slacks.emplace_back(r, slackCol, value);

		if (type == CONSTRAINT_TYPE::LESS) {
			initialBasisEntries.emplace_back(r, slackCol);
		}

		slackCol++;
		_slacksCount++;
	}
}

// Builds the Aritficials vector
void MPSParser::buildArtificials() {
	int artificialCol = _columnIndex + (int)slacks.size();
	for (int r = 0; r < getRowCount(); r++) {
		auto& constraintType = constraintMapRaw[r];

		if (constraintType == CONSTRAINT_TYPE::FREE ||
			constraintType == CONSTRAINT_TYPE::LESS) continue;

		double value = 1;
		artificials.emplace_back(r, artificialCol, value);

		initialBasisEntries.emplace_back(r, artificialCol);
		artificialCol++;
		_artificialCount++;
	}

}

// Builds the sparse vector for rhs values from rhsColumn
void MPSParser::buildEigenSparseVectors() {
	Eigen::VectorXd denseRhs = Eigen::VectorXd::Zero(static_cast<Eigen::Index>(getRowCount()));
	for (const auto& t : rhsColumn) {
		denseRhs(t.row()) = t.value();
	}
	b = denseRhs.sparseView();

	c.resize(getColumnCount());
	for (const auto& t : objectiveRow) {
		c.insert(t.col()) = t.value();
	}
}

// Sorts the entries, by row,
// (The entries are already sorted by column, after reading from the MPS file)
void MPSParser::sortEntries() {
	using T = Eigen::Triplet<double>;
	std::ranges::sort(columnSectionEntries, [](const T& a, const T& b) {
		if (a.col() != b.col()) return a.col() < b.col();
		return a.row() < b.row();
	});

	std::ranges::sort(rhsColumn, [](const T& a, const T& b) {
		return a.row() < b.row();
	});

	std::ranges::sort(initialBasisEntries, [](const std::pair<std::size_t, std::size_t>& a, const std::pair<std::size_t, std::size_t>& b) {
		return a.first < b.first;
	});
}

// helper function that removes the whitespace from first word
std::string_view MPSParser::_firstWord(std::string_view line)
{
	// Find the first character that is not a space or tab
	const auto start = line.find_first_not_of(" \t");
	
	if (start == std::string_view::npos)
		return {};

	// Find the end of the first word
	const auto end = line.find_first_of(" \t", start);

	if (end == std::string_view::npos)
		return line.substr(start);

	return line.substr(start, end - start);
}

void MPSParser::_processLine(std::string_view line)
{
    const std::string_view first_word = _firstWord(line);

    if (_isSection(line)) {
        curr_section = _getSection(line);
    }
    else if (curr_section == MPSDATA_LINE::ROWS) {
        auto row = _catchConstraint(line);
        constraintMap.insert({ row.second, row.first });
    }
    else if (curr_section == MPSDATA_LINE::COLUMNS) {
        _catchColumn(line);
    }
	else if (curr_section == MPSDATA_LINE::RHS) {
		_catchRHS(line);
	}
}


bool MPSParser::_hasColumn(std::string_view name)
{
	return columnMap.find(std::string(name)) != columnMap.end();
}


void MPSParser::_catchColumn(std::string_view line)
{
	const std::string columnName = std::string(_firstWord(line));

	if (columnName.empty()) {
		return;
	}

	if (!_hasColumn(columnName)) {
		columnMap[columnName] = _columnIndex++;
	}

	// Parse the rest of the line
	auto pos = line.find_first_not_of(" \t");

	if (pos == std::string_view::npos)
		return;

	// Skip the first word
	pos = line.find_first_of(" \t", pos);

	if (pos == std::string_view::npos)
		return;

	std::istringstream iss(std::string(line.substr(pos)));

	std::string row_name;
	std::string row_value;

	while (iss >> row_name >> row_value) {
		double v = std::stod(row_value);
		const auto constraint = constraintMap.find(row_name);
		if (constraint == constraintMap.end()) {
			throw std::runtime_error("Unknown row in COLUMNS section: " + row_name);
		}

		const bool isObjectiveRow = constraint->second == CONSTRAINT_TYPE::FREE;

		if (isObjectiveRow) {
			objectiveRow.push_back(Eigen::Triplet<double>(
				rowMap[row_name],
				columnMap[columnName],
				v
			));
		}
		else {
			columnSectionEntries.push_back(Eigen::Triplet<double>(
				rowMap[row_name],
				columnMap[columnName],
				v
			));
		}

	}
}

void MPSParser::_catchRHS(std::string_view line) {
	const std::string columnName = std::string(_firstWord(line));

	if (columnName.empty()) {
		return;
	}

	// Parse the rest of the line
	auto pos = line.find_first_not_of(" \t");

	if (pos == std::string_view::npos)
		return;

	// Skip the first word
	pos = line.find_first_of(" \t", pos);

	if (pos == std::string_view::npos)
		return;

	std::istringstream iss(std::string(line.substr(pos)));

	std::string row_name;
	std::string row_value;

	while (iss >> row_name >> row_value) {
		double v = std::stod(row_value);
		const auto constraint = constraintMap.find(row_name);
		if (constraint == constraintMap.end()) {
			throw std::runtime_error("Unknown row in RHS section: " + row_name);
		}
		// An RHS entry for the objective is its constant term, not a constraint RHS.
		if (constraint->second == CONSTRAINT_TYPE::FREE) continue;
		rhsColumn.emplace_back(static_cast<int>(rowMap[row_name]), 0, v);
	}
}

// The initial slack/artificial basis is feasible only when b >= 0.  Convert
// rows with a negative RHS to an equivalent row with a non-negative RHS.
void MPSParser::normalizeConstraintSigns() {
	std::vector<double> rhs(getRowCount(), 0.0);
	for (const auto& entry : rhsColumn) {
		rhs[static_cast<std::size_t>(entry.row())] = entry.value();
	}

	for (std::size_t row = 0; row < getRowCount(); ++row) {
		if (rhs[row] >= 0.0) continue;

		auto& constraint = constraintMapRaw[row];
		if (constraint == CONSTRAINT_TYPE::LESS) {
			constraint = CONSTRAINT_TYPE::GREATER;
		}
		else if (constraint == CONSTRAINT_TYPE::GREATER) {
			constraint = CONSTRAINT_TYPE::LESS;
		}

		for (auto& entry : columnSectionEntries) {
			if (entry.row() == static_cast<int>(row)) {
				entry = Eigen::Triplet<double>(entry.row(), entry.col(), -entry.value());
			}
		}
		for (auto& entry : rhsColumn) {
			if (entry.row() == static_cast<int>(row)) {
				entry = Eigen::Triplet<double>(entry.row(), entry.col(), -entry.value());
			}
		}
	}
}

