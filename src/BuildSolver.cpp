#include <utility>

#include "ESolverAlgorithm.hpp"
#include "BuildSolver.hpp"
#include "LPSolver.hpp"

BuildSolver::BuildSolver() {}

BuildSolver& BuildSolver::setRows(std::size_t r) {
	rows_ = r;
	return *this;
}

BuildSolver& BuildSolver::setColumns(std::size_t c) {
	cols_ = c;
	return *this;
}

BuildSolver& BuildSolver::setSlackCount(std::size_t s) {
	slackCount_ = s;
	return *this;
}
BuildSolver& BuildSolver::setArtificialCount(std::size_t a) {
	artificialCount_ = a;
	return *this;

}

BuildSolver& BuildSolver::setObjectiveRow(Eigen::SparseVector<double> objCoeff) {
	c_ = std::move(objCoeff);
	return *this;
}

BuildSolver& BuildSolver::setRHS(Eigen::SparseVector<double> rhs) {
	xB_ = std::move(rhs);
	return *this;
}

BuildSolver& BuildSolver::setColumnEntries(std::vector<Eigen::Triplet<double>> entries) {
	columnEntries_ = std::move(entries);
	return *this;
}

BuildSolver& BuildSolver::setSlacks(std::vector<Eigen::Triplet<double>> s) {
	slacks_ = std::move(s);
	return *this;
}

BuildSolver& BuildSolver::setArtificials(std::vector<Eigen::Triplet<double>> a) {
	artificials_ = std::move(a);
	return *this;
}

BuildSolver& BuildSolver::setInitialBasisColumn(std::vector<std::pair<std::size_t, std::size_t>> b) {
	initialBasisEntries_ = std::move(b);
	return *this;
}

LPSolver BuildSolver::build() {
	return {
		rows_,
		cols_,
		slackCount_,
		artificialCount_,
		std::move(c_),
		std::move(xB_),
		std::move(columnEntries_),
		std::move(slacks_),
		std::move(artificials_),
		std::move(initialBasisEntries_),
		SolverAlgorithm::LP_REVISED_SIMPLEX
	};
}
