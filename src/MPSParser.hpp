#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <filesystem>

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

struct StringHash {
	using is_transparent = void; // Enables heterogeneous lookup
	using hash_type = std::hash<std::string_view>;

	size_t operator()(std::string_view sv) const noexcept {
		return hash_type{}(sv);
	}
};

// Custom equality comparator for std::string and std::string_view
struct StringEqual {
	using is_transparent = void; // Enables heterogeneous lookup

	bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
		return lhs == rhs;
	}
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

	enum class CONSTRAINT_TYPE : std::uint8_t {
		EQUAL = 0,
		LESS = 1,
		GREATER = 2,
		FREE = 3  // States the objective function
	};
public:
	MPSParser(const std::filesystem::path& path);

	int getRowCount() {
		return _rowIndex;
	}
	int getColumnCount() {
		return _columnIndex;
	}
	const std::vector<Entry>& getEntries() const& { return this->entries; }
private:
	std::string_view _firstWord(std::string_view line);
	void _processLine(std::string_view line);
	bool _hasColumn(std::string_view name);
	bool _isSection(std::string_view line);
	void _catchColumn(std::string_view line);
	MPSDATA_LINE _getSection(std::string_view line);
	std::pair<MPSParser::CONSTRAINT_TYPE, std::string> _catchConstraint(std::string_view line);


private:
	int _columnIndex = 0;
	int _rowIndex = 0;
	MPSDATA_LINE curr_section = MPSDATA_LINE::VOID;


	// Stores a particular row against that row's constraint type
	std::unordered_map<std::string, CONSTRAINT_TYPE, StringHash, StringEqual> constraintMap;

	// Stores the rows against their index staring from 0;
	std::unordered_map<std::string, int, StringHash, StringEqual> rowMap;

	// Stores the columns against their index staring from 0;
	std::unordered_map<std::string, int, StringHash, StringEqual> columnMap;

	std::vector<Entry> entries;
};


