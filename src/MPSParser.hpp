#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <functional>
#include <filesystem>
#include <cstdint>
#include "types.hpp"

#include <Eigen/Sparse>
#include <Eigen/Core>


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

	std::size_t getRowCount() const noexcept { return _rowIndex; }
	std::size_t getColumnCount() const noexcept { return _columnIndex; }
	std::size_t getSlacksCount() const noexcept { return _slacksCount; }
	std::size_t getArtificialsCount() const noexcept { return _artificialCount; }

	Eigen::SparseVector<double> takeObjectiveRow() noexcept { return std::move(this->c); }
	Eigen::SparseVector<double> takeRHSColumn() noexcept { return std::move(this->b); }
	std::vector<Eigen::Triplet<double>> takeColumnEntries() noexcept { return std::move(this->columnSectionEntries); }
	std::vector<Eigen::Triplet<double>> takeSlackEntries() noexcept { return std::move(this->slacks); }
	std::vector<Eigen::Triplet<double>> takeArtificialEntries() noexcept { return std::move(this->artificials); }
	std::vector<std::pair<std::size_t, std::size_t>> takeInitialBasisEntries() noexcept { return std::move(this->initialBasisEntries); }
private:
	std::string_view _firstWord(std::string_view line);
	void _processLine(std::string_view line);
	bool _hasColumn(std::string_view name);
	bool _isSection(std::string_view line);
	void _catchColumn(std::string_view line);
	MPSDATA_LINE _getSection(std::string_view line);
	std::pair<MPSParser::CONSTRAINT_TYPE, std::string> _catchConstraint(std::string_view line);
	void _catchRHS(std::string_view line);

	void buildSlacks();
	void buildArtificials();
	void normalizeConstraintSigns();
	void buildEigenSparseVectors();
	void sortEntries();

private:
	std::size_t _columnIndex = 0;
	std::size_t _rowIndex = 0;

	std::size_t _slacksCount = 0;
	// Aritficials column count
	std::size_t _artificialCount = 0;

	// Stores the number of constraints containing equality (=)
	int _equalityConstraints = 0;
	// Stores the number of constraints containing inequality less than (<=) or greather then (>=)
	int _inequalityConstraints = 0;

	MPSDATA_LINE curr_section = MPSDATA_LINE::VOID;


	// Stores a particular row against that row's constraint type
	std::unordered_map<std::string, CONSTRAINT_TYPE, StringHash, StringEqual> constraintMap;

	// Stores a particular row indice against that row's constraint type
	std::unordered_map<std::size_t, CONSTRAINT_TYPE> constraintMapRaw;

	// Stores the rows against their index staring from 0;
	std::unordered_map<std::string, std::size_t, StringHash, StringEqual> rowMap;

	// Stores the columns against their index staring from 0;
	std::unordered_map<std::string, std::size_t, StringHash, StringEqual> columnMap;

	// Stores the entries of the column section
	std::vector<Eigen::Triplet<double>> columnSectionEntries;

	// Stores the values of the objective row
	std::vector<Eigen::Triplet<double>> objectiveRow;

	// Stores the objective coefficients in Eigen::SparseVector from objectiveRow
	Eigen::SparseVector<double> c;

	// Stores the RHS values
	std::vector<Eigen::Triplet<double>> rhsColumn;

	// Stores the rhs values in Eigen::SparseVector from rhsColumn
	Eigen::SparseVector<double> b;

	// Stores the Slacks(Surplus)
	std::vector<Eigen::Triplet<double>> slacks;

	// Stores the Aritificials
	std::vector<Eigen::Triplet<double>> artificials;

	// Stores the initial basis columns
	std::vector<std::pair<std::size_t, std::size_t>> initialBasisEntries;

};


