
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <array>
#include <regex>

#include "MPSParser.hpp"

MPSParser::MPSParser(const std::string& filepath) 
{
	std::ifstream file(filepath);
	
	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filepath);
	}

	std::string line;

	MPSDATA_LINE curr_section = MPSDATA_LINE::VOID;
	bool store = false;

	while (std::getline(file, line)) {
		if (_isSection(line)) {
			curr_section = _getSection(line);
			continue;
		}

		if (curr_section == MPSDATA_LINE::ROWS) {
			auto row = _catchConstraint(line);
			constraint_map.insert({ row.second, row.first });
		}
		else if (curr_section == MPSDATA_LINE::COLUMNS) {
			_catchColumn(line);
		}

	}

	file.close();
}

bool MPSParser::_isSection(const std::string& line) {
	std::istringstream iss(line);
	std::string firstWord;

	iss >> firstWord;

	if (firstWord == "NAME" ||
		firstWord == "ROWS" || 
		firstWord == "COLUMNS" ||
		firstWord == "RHS" ||
		firstWord == "BOUNDS"||
		firstWord == "ENDATA") {

		return true;
	}
	else {
		return false;
	}
}

MPSParser::MPSDATA_LINE MPSParser::_getSection(const std::string& line) {
	using T = MPSParser::MPSDATA_LINE;

	std::istringstream iss(line);
	std::string firstWord;

	iss >> firstWord;

	if (firstWord == "NAME") {
		return T::NAME;
	}
	else if (firstWord == "ROWS") {
		return T::ROWS;
	}
	else if (firstWord == "COLUMNS") {
		return T::COLUMNS;
	}
	else if (firstWord == "RHS") {
		return T::RHS;
	}
	else if (firstWord == "BOUNDS") {
		return T::BOUNDS;
	}
	else if (firstWord == "ENDATA") {
		return T::ENDATA;
	}

}

std::pair<MPSParser::CONSTRAINT_TYPE, std::string>
	MPSParser::_catchConstraint(const std::string& line)
{
	using constraint_type_t = MPSParser::CONSTRAINT_TYPE;

	std::istringstream iss(line);
	std::string constraint_str;
	std::string row_name;

	constraint_type_t  _constraint_type = constraint_type_t::FREE;

	iss >> constraint_str >> row_name;

	if (constraint_str == "E")
	{
		_constraint_type = constraint_type_t::EQUAL;
	}
	else if (constraint_str == "L")
	{
		_constraint_type = constraint_type_t::LESS;
	}
	else if (constraint_str == "G")
	{
		_constraint_type = constraint_type_t::GREATER;
	}
	else if (constraint_str == "N")
	{
		_constraint_type = constraint_type_t::FREE;
	}

	row_map[row_name] = _row_index++;
	return {_constraint_type, row_name};
}

bool MPSParser::_isColumn(const std::string& name) {
	auto it = column_map.find(name);
	if (it != column_map.end()) return true;
	else return false;
}

void MPSParser::_catchColumn(const std::string& line) {
	std::istringstream iss(line);

	std::string column_name;

	iss >> column_name;
	if (!_isColumn(column_name)) {
		column_map[column_name] = _column_index++;
	}

	std::string row_name, row_value;
	while (iss >> row_name >> row_value) {
		Value v = row_value;
		entries.push_back({ row_map[row_name], column_map[column_name], v });
	}

}

