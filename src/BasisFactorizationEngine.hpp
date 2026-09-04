#pragma once

#include <cstdint>
#include <utility>
#include <Eigen/Sparse>

// Basis Factorization Engine
class BFE {
	using T = Eigen::Triplet<double>;
public:
	BFE() = default;
	void initialize(
		std::size_t r,
		std::size_t c,
		const Eigen::SparseMatrix<double>& p,
		const std::vector<std::pair<std::size_t, std::size_t>>& iB
	);

	void pivot(std::size_t enteringCol, std::size_t leavingCol);
	void factorize();

	Eigen::SparseLU<Eigen::SparseMatrix<double>>& getBasisLU();
	const std::vector<int>& getBasisIndices() const noexcept { return this->basisIndices_; };
private:
	void buildInitialBasis();

	std::size_t rows_ = 0;
	std::size_t cols_ = 0;

	std::vector<int> basisIndices_;

	const Eigen::SparseMatrix<double>* parentMat_ = nullptr;
	const std::vector<std::pair<std::size_t, std::size_t>>* initialBasisColumns_ = nullptr;

	Eigen::SparseLU<Eigen::SparseMatrix<double>> basis_;
	Eigen::SparseMatrix<double> B_;

};
