#pragma once

#include <utility>
#include <unordered_map>
#include <fstream>
#include <string>

struct Value {
private:
	std::string value = "";

	enum class SIGN {PLUS, MINUS} sign = SIGN::PLUS;
	enum class TYPE {M_INT, M_DOUBLE} type = TYPE::M_DOUBLE;

	double abs_value = 0; // absolute value
public:

	Value() = default;

	Value(const std::string& str) {
		*this = str;
	}

	Value& operator=(const std::string& str) {
		value = str;
		auto v = std::stod(str);
		if (v >= 0) {
			sign = SIGN::PLUS;
			abs_value = static_cast<double>(v);
		}
		else if (v < 0) {
			sign = SIGN::MINUS;
			abs_value = static_cast<double>(-v);
		}

		auto t = v - static_cast<long int>(v);
		if (t != 0) type = TYPE::M_DOUBLE;
		else type = TYPE::M_INT;

		return *this;

	}

	bool isPositive() const {
		return sign == SIGN::PLUS ? true : false;
	}

	bool flipSign() {
		if (sign == SIGN::PLUS) sign = SIGN::MINUS;
		else sign = SIGN::PLUS;
	}

	double getValue() {
		return isPositive()
			? static_cast<double>(abs_value)
			: -static_cast<double>(abs_value);
	}
};

struct Entry {
	int row;
	int col;
	Value value;
};

class MPSParser {
public:
	// This enum class defines the MPS syntax into chunks
	enum class MPSDATA_LINE : std::uint8_t {
		VOID = 0,
		NAME = 1,
		ROWS = 2,
		COLUMNS = 3,
		RHS = 4,
		BOUNDS = 5,
		ENDATA = 6
	};

	enum class CONSTRAINT_TYPE: std::uint8_t {
		EQUAL = 0,
		LESS = 1,
		GREATER = 2,
		FREE = 3  // States the objective function
	};

public:
	MPSParser(const std::string& filepath);

	int getRowCount() {
		return _row_index;
	}

	int getColumnCount() {
		return _column_index;
	}
private:
	int _column_index = 0;
	int _row_index = 0;

	// Stores a particular row against that row's constraint type
	std::unordered_map<std::string, CONSTRAINT_TYPE > constraint_map;

	// Stores the rows against their index staring from 0;
	std::unordered_map<std::string, int> row_map;

	// Stores the columns against their index staring from 0;
	std::unordered_map<std::string, int> column_map;

	std::vector<Entry> entries;

	bool _isSection(const std::string& line);
	MPSDATA_LINE _getSection(const std::string& line);
	std::pair<CONSTRAINT_TYPE, std::string> _catchConstraint(const std::string& line);
	void _catchColumn(const std::string& line);

	bool _isColumn(const std::string& name);
};