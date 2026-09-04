#include <iostream>
#include <cstdint>
#include <cassert>
#include <utility>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>

#include "BasisFactorizationEngine.hpp"

void BFE::initialize(
	std::size_t r,
	std::size_t c,
	const Eigen::SparseMatrix<double>& p,
	const std::vector<std::pair<std::size_t, std::size_t>>& iB
) {
	rows_ = (r);
	cols_ = (c);
	parentMat_ = &p;
	initialBasisColumns_ = &iB;

	//assert(rows_ == cols_);
	
	buildInitialBasis();
	//factorize();
	basis_.analyzePattern(B_);
	basis_.factorize(B_);
}

void BFE::buildInitialBasis() {

	B_.resize(rows_, rows_);

	std::vector<T> basisEntries;
	basisEntries.reserve(rows_);

	int j = 0;
	for (const auto& [row, colInA] : *initialBasisColumns_) {
		for (Eigen::SparseMatrix<double>::InnerIterator it(*parentMat_, colInA); it; ++it) {
			basisEntries.emplace_back(it.row(), j, it.value());
		}
		j++;
	}

	B_.setFromTriplets(basisEntries.begin(), basisEntries.end());

	basisIndices_.resize(rows_);
	int i = 0;
	for (const auto& [row, col] : *initialBasisColumns_) {
		basisIndices_[i] = col;
		i++;
	}
}

void BFE::pivot(std::size_t enteringColIndex, std::size_t leavingColIndex) {
	B_.col(leavingColIndex) = (*parentMat_).col(enteringColIndex);
	basisIndices_[leavingColIndex] = enteringColIndex;
}

void BFE::factorize() {
	basis_.compute(B_);
}

Eigen::SparseLU<Eigen::SparseMatrix<double>>& BFE::getBasisLU() {
	return basis_;
}