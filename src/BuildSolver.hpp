#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include <Eigen/Sparse>

#include "LPSolver.hpp"

class BuildSolver {
	std::size_t rows_ = 0;
	std::size_t cols_ = 0;

	std::size_t slackCount_ = 0;
	std::size_t artificialCount_ = 0;

	// Objective row coefficients sparse vector
	Eigen::SparseVector<double> c_;
	// RHS Values sparse vector
	Eigen::SparseVector<double> xB_;
	std::vector<Eigen::Triplet<double>> columnEntries_;
	std::vector<Eigen::Triplet<double>> slacks_;
	std::vector<Eigen::Triplet<double>> artificials_;
	
	std::vector<std::pair<std::size_t, std::size_t>> initialBasisEntries_;
public:
	BuildSolver();

	BuildSolver& setRows(std::size_t);
	BuildSolver& setColumns(std::size_t);
	BuildSolver& setSlackCount(std::size_t);
	BuildSolver& setArtificialCount(std::size_t);
	BuildSolver& setObjectiveRow(Eigen::SparseVector<double>);
	BuildSolver& setRHS(Eigen::SparseVector<double>);
	BuildSolver& setColumnEntries(std::vector<Eigen::Triplet<double>>);
	BuildSolver& setSlacks(std::vector<Eigen::Triplet<double>>);
	BuildSolver& setArtificials(std::vector<Eigen::Triplet<double>>);
	BuildSolver& setInitialBasisColumn(std::vector<std::pair<std::size_t, std::size_t>>);

	LPSolver build();

};