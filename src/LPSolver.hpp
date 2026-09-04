#pragma once

#include <utility>
#include <cstdint>
#include <Eigen/sparse>

#include "ESolverAlgorithm.hpp"
#include "BasisFactorizationEngine.hpp"

class LPSolver {
	using SV = Eigen::SparseVector<double>;
	using T = Eigen::Triplet<double>;
public:

	LPSolver(std::size_t rows, std::size_t cols, std::size_t slacks, std::size_t artificials, 
		SV c, SV xB, 
		std::vector<T> columnEntries,
		std::vector<T> slackEntries,
		std::vector<T> artificialEntries,
		std::vector<std::pair<std::size_t, std::size_t>> initialBasisEntries,
		SolverAlgorithm algo_);

	void solve();

private:

	void buildConstraintCoefficientMatrix();
	void removeArtificialBasics();
	void solveSimplex(double& objVal, const SV& costVector, std::size_t enteringColumnCount, bool printSolution);
	double extractSolution(const SV& costVector, bool printSolution);
	std::size_t totalColumnCount() const noexcept;

	std::size_t rows_;
	std::size_t cols_;

	std::size_t slackCount_;
	std::size_t artificialCount_;

	SolverAlgorithm algo_ = SolverAlgorithm::LP_REVISED_SIMPLEX;

	// Objective row coefficients sparse vector
	SV c_;
	// RHS Values sparse vector
	SV xB_;
	// Constraint coefficients
	std::vector<T> columnEntries_;
	// Slacks/Surplus
	std::vector<T> slackEntries_;
	// Artificials
	std::vector<T> artificialEntries_;
	// Initial basis matrix column indices <row, column>
	std::vector<std::pair<std::size_t,std::size_t>> initialBasisEntries_;
	
	// Constraint coefficient matrix
	Eigen::SparseMatrix<double> A_;

	// Basis Matrix (Basis Factorization Engine)
	BFE bfe_;

};
