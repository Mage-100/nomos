#include <iostream>
#include <cstdint>
#include <cassert>
#include <print>
#include <stdexcept>
#include <limits>
#include <Eigen/Sparse>
#include <Eigen/LU>
#include <Eigen/SparseLU>

#include "LPSolver.hpp"
#include "ESolverAlgorithm.hpp"
#include "BasisFactorizationEngine.hpp"

LPSolver::LPSolver(
	std::size_t rows, 
	std::size_t cols,
	std::size_t slacks, 
	std::size_t artificials,
	SV c, 
	SV xB,
	std::vector<T> columnEntries,
	std::vector<T> slackEntries,
	std::vector<T> artificialEntries,
	std::vector<std::pair<std::size_t, std::size_t>> initialBasisEntries,
	SolverAlgorithm algo
) : 
	rows_(std::move(rows)), 
	cols_(std::move(cols)),
	slackCount_(slacks), 
	artificialCount_(artificials), 
	c_(std::move(c)),
	xB_(std::move(xB)),
	columnEntries_(std::move(columnEntries)),
	slackEntries_(std::move(slackEntries)),
	artificialEntries_(std::move(artificialEntries)),
	initialBasisEntries_(std::move(initialBasisEntries)),
	algo_(algo)
{
	buildConstraintCoefficientMatrix();

	assert(initialBasisEntries_.size() == rows_);
	bfe_.initialize(
		rows_, 
		rows_,
		A_,
		initialBasisEntries_
	);

}
void LPSolver::buildConstraintCoefficientMatrix() {
	A_.resize(rows_, totalColumnCount());

	const auto insert = [](std::vector<T>& m, std::vector<T>& e){
		m.insert(m.end(), e.begin(), e.end());
	};
	
	std::vector<T> matrixEntries;
	matrixEntries.reserve(columnEntries_.size() + slackEntries_.size() + artificialEntries_.size());
	insert(matrixEntries, columnEntries_);
	insert(matrixEntries, slackEntries_);
	insert(matrixEntries, artificialEntries_);

	A_.setFromTriplets(matrixEntries.begin(), matrixEntries.end());
}

std::size_t LPSolver::totalColumnCount() const noexcept {
	return cols_ + slackCount_ + artificialCount_;
}

void LPSolver::removeArtificialBasics() {
	constexpr double tolerance = 1e-9;
	const std::size_t nonArtificialColumnCount = cols_ + slackCount_;

	for (std::size_t basisPosition = 0; basisPosition < rows_; ++basisPosition) {
		auto& B = bfe_.getBasisLU();
		const auto& basisIndices = bfe_.getBasisIndices();
		if (basisIndices[basisPosition] < static_cast<int>(nonArtificialColumnCount)) continue;

		const Eigen::VectorXd xB = B.solve(Eigen::VectorXd(xB_));
		if (std::abs(xB(static_cast<Eigen::Index>(basisPosition))) > tolerance) {
			throw std::runtime_error("Problem is infeasible: an artificial variable remains positive after Phase I.");
		}

		int enteringColumn = -1;
		for (std::size_t candidate = 0; candidate < nonArtificialColumnCount; ++candidate) {
			bool isBasic = false;
			for (const int basisColumn : basisIndices) {
				if (basisColumn == static_cast<int>(candidate)) {
					isBasic = true;
					break;
				}
			}
			if (isBasic) continue;

			const Eigen::VectorXd direction = B.solve(
				Eigen::VectorXd(A_.col(static_cast<Eigen::Index>(candidate))));
			if (std::abs(direction(static_cast<Eigen::Index>(basisPosition))) > tolerance) {
				enteringColumn = static_cast<int>(candidate);
				break;
			}
		}

		if (enteringColumn == -1) {
			throw std::runtime_error(
				"Phase I found a redundant constraint. Removing redundant rows is not implemented yet.");
		}

		bfe_.pivot(static_cast<std::size_t>(enteringColumn), basisPosition);
		bfe_.factorize();
	}
}

void LPSolver::solveSimplex(
	double& objVal,
	const SV& costVector,
	std::size_t enteringColumnCount,
	bool printSolution
) {
	constexpr double tolerance = 1e-9;
	constexpr int maximumIterations = 100000;
	const std::size_t nConstraints = rows_;
	int iterations = 0;

	while (true) {

		// Step - 1
		// Creating the basis matrix - every iteration
		auto& B = bfe_.getBasisLU();
		const auto& basisIndices = bfe_.getBasisIndices();

		// Step - 2
		// Calculating revised RHS = x_B
		// Solve B * x_B = b  ->  x_B = B^-1 * b
		if (B.info() != Eigen::Success) {
			throw std::runtime_error("Basis factorization failed.");
		}
		Eigen::VectorXd x_B = B.solve(Eigen::VectorXd(xB_));

		// Step - 3 - Calculating B*T * lambda = c_B -> simplex multipliers
		// c_n` = c_n - (c_B^T * B^-1) * P_n
		// Substiuting
		// c_B^T * B^-1 = (B^-T * c_B)^T = (lambda)^T
		// lambda = B^-T * c_B

		// Use LU factorisation to solve for lambda
		// B^T * lambda = c_B
		// c_n` = c_n - lambda^T * P_n

		Eigen::VectorXd c_B(nConstraints);
		for (int i = 0; i < nConstraints; i++) {
			c_B(i) = costVector.coeff(basisIndices[i]);
		}

		Eigen::VectorXd lambda = B.transpose().solve(c_B);

		// Step - 4 - Computed reduced costs -> c_n` , find entering variable
		// For maximization - find the most positive reduced cost
		// For minimization - find the most negative reduced cost
		int enteringCol = -1;
		for (std::size_t i = 0; i < enteringColumnCount; ++i) {
			// Only iterate for non-basic columns
			bool isBasic = false;
			for (int idx : basisIndices) if (idx == i) { isBasic = true; break; }
			if (isBasic) continue;

			double reducedCost = costVector.coeff(static_cast<Eigen::Index>(i)) -
				lambda.dot(Eigen::VectorXd(A_.col(static_cast<Eigen::Index>(i))));

			// Bland's rule avoids cycling on degenerate bases.  This is a
			// minimization simplex implementation, hence a negative reduced cost enters.
			if (reducedCost < -tolerance) {
				enteringCol = i;
				break;
			}
		}

		// Step - 5 - If no negative reduced cost -> optimal
		if (enteringCol == -1) break;

		// Step - 6 - Calculating direction: d = B^-1 * a_entering
		Eigen::VectorXd d = B.solve(Eigen::VectorXd(A_.col(enteringCol)));

		// Step - 7 - Min-ratio test to find leaving variable
		int leavingPos = -1;
		double minRatio = std::numeric_limits<double>::infinity();

		for (int i = 0; i < nConstraints; i++) {
			if (d(i) > tolerance) {
				double ratio = x_B(i) / d(i);
				if (ratio < minRatio) {
					minRatio = ratio;
					leavingPos = i;
				}
			}
		}

		// Step - 8 Unboundedness check
		if (leavingPos == -1) {
			throw std::runtime_error("Problem is unbounded.");
		}

		// Step - 9 Pivot: swap entering column into basis at leavingPos
		bfe_.pivot(enteringCol, leavingPos);
		bfe_.factorize();
		if (++iterations >= maximumIterations) {
			throw std::runtime_error("Simplex iteration limit reached.");
		}

	}
	std::println("Iterations took: {}", iterations);

	objVal = extractSolution(costVector, printSolution);
}

double LPSolver::extractSolution(const SV& costVector, bool printSolution) {
	// Extract solution
	auto& B = bfe_.getBasisLU();
	auto& basisIndices = bfe_.getBasisIndices();
	Eigen::VectorXd x_B_final = B.solve(Eigen::VectorXd(xB_));

	Eigen::VectorXd x_opt = Eigen::VectorXd::Zero(totalColumnCount());
	for (std::size_t i = 0; i < rows_; ++i) {
		x_opt(basisIndices[i]) = x_B_final(i);
	}

	if (printSolution) {
		std::println("\nOptimal Solution:");
		for (std::size_t i = 0; i < cols_; ++i) {
			std::println(" x[{}] = {:.4f}", i + 1, x_opt(i));
		}
	}

	double objVal = costVector.dot(x_opt);
	if (printSolution) {
		const double residual = (A_ * x_opt - Eigen::VectorXd(xB_)).lpNorm<Eigen::Infinity>();
		double largestArtificial = 0.0;
		for (std::size_t i = cols_ + slackCount_; i < totalColumnCount(); ++i) {
			largestArtificial = std::max(largestArtificial, std::abs(x_opt(static_cast<Eigen::Index>(i))));
		}
		std::println("\nObjective value: {:.15f}", objVal);
		//std::println("Maximum primal residual: {:.3e}; largest artificial: {:.3e}", residual, largestArtificial);
	}
	return objVal;
}

void LPSolver::solve() {
	constexpr double tolerance = 1e-8;
	const std::size_t nonArtificialColumnCount = cols_ + slackCount_;

	// Phase I minimizes the sum of artificial variables.
	if (artificialCount_ > 0) {
		SV phaseOneCost(totalColumnCount());

		for (std::size_t i = nonArtificialColumnCount; i < totalColumnCount(); ++i) {
			phaseOneCost.insert(static_cast<Eigen::Index>(i)) = 1.0;
		}

		double phaseOneObjective = 0.0;
		solveSimplex(phaseOneObjective, phaseOneCost, totalColumnCount(), false);
		std::println("Phase 1 objective value: {:.15f}", phaseOneObjective);
		if (phaseOneObjective > tolerance) {
			throw std::runtime_error("Problem is infeasible.");
		}
		removeArtificialBasics();
	}

	// Preserve the augmented matrix dimension, but Phase II starts with no
	// artificial basic variable and artificial columns are barred from entering.
	SV phaseTwoCost(totalColumnCount());
	for (SV::InnerIterator it(c_); it; ++it) {
		phaseTwoCost.insert(it.index()) = it.value();
	}

	double objective = 0.0;
	solveSimplex(objective, phaseTwoCost, nonArtificialColumnCount, true);
}
