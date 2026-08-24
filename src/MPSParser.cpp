#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <array>
#include <filesystem>
#include "MPSParser.hpp"
#include <string_view>

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
}

bool MPSParser::_isSection(std::string_view line)
{
	const std::string_view firstWord = _firstWord(line);

	// Checking whether the first word is a valid MPS section.
	return firstWord == "NAME" ||
		firstWord == "ROWS" ||
		firstWord == "COLUMNS" ||
		firstWord == "RHS" ||
		firstWord == "BOUNDS" ||
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
			break;
		case 'L':
			constraintType = ConstraintType::LESS;
			break;
		case 'G':
			constraintType = ConstraintType::GREATER;
			break;
		case 'N':
			constraintType = ConstraintType::FREE;
			break;
		}
	}

	// Stores the row name with incrementing the index value.
	rowMap[rowName] = _rowIndex++;

	return { constraintType, rowName };
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
		Value v = row_value;

		entries.push_back({
			rowMap[row_name],
			columnMap[columnName],
			v
			});
	}
}



