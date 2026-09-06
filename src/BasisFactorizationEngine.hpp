#pragma once

#include <cstdint>
#include <utility>
#include <memory>

#include <Eigen/Sparse>

// Basis Factorization Engine
template <typename Scalar>
class BFE {
    using SparseMatrix = Eigen::SparseMatrix<Scalar>;
    using SparseLU = Eigen::SparseLU<SparseMatrix>;
    using Triplet = Eigen::Triplet<Scalar>;

public:
    BFE() = default;

    void initialize(const SparseMatrix& constraintMatrix, const std::vector<std::pair<std::size_t, std::size_t>>& initialBasisColumns);
    void pivot(std::size_t enteringColumn, std::size_t leavingPosition);
    void factorize();

    SparseLU& getBasisLU() noexcept { return basisLU_;}

    const std::vector<std::size_t>& getBasisColumnIndices() const noexcept { return basisColumnIndices_; }

private:
    void buildInitialBasis(const std::vector<std::pair<std::size_t, std::size_t>>& initialBasisColumns);

    std::size_t numRows_{ 0 };
    std::size_t numCols_{ 0 };

    // Maps basis position -> column in constraintMatrix_
    std::vector<std::size_t> basisColumnIndices_;

    // Original constraint matrix A
    const SparseMatrix* constraintMatrix_{ nullptr };

    // Current basis matrix B
    SparseMatrix basisMatrix_;

    // LU factorization of B
    SparseLU basisLU_;
};

#include <iostream>
#include <cassert>
#include <Eigen/SparseLU>
#include <exception>
#include <stdexcept>

template<typename Scalar>
void BFE<Scalar>::initialize(const SparseMatrix& constraintMatrix, const std::vector<std::pair<std::size_t, std::size_t>>& initialBasisColumns)
{
    const auto r = constraintMatrix.rows();
    const auto c = constraintMatrix.cols();

    if (r == 0 || c == 0) {
        throw std::invalid_argument(
            "BFE::initialize: matrix dimensions must be greater than zero"
        );
    }

    if (constraintMatrix.rows() != static_cast<Eigen::Index>(r) || constraintMatrix.cols() != static_cast<Eigen::Index>(c)) {
        throw std::invalid_argument("[BFS::initialize]: matrix dimensions must be greater than zero");
    }

    if (initialBasisColumns.size() != r) {
        throw std::invalid_argument(
            "BFE::initialize: number of basis columns must equal number of rows"
        );
    }

    constraintMatrix_ = &constraintMatrix;

    numRows_ = r;
    numCols_ = c;
    //assert(numRows_ == numCols_);

    buildInitialBasis(initialBasisColumns);
    //factorize();
    basisLU_.analyzePattern(basisMatrix_);
    basisLU_.factorize(basisMatrix_);
}

template <typename Scalar>
void BFE<Scalar>::buildInitialBasis(
    const std::vector<std::pair<std::size_t, std::size_t>>& initialBasisColumns
)
{
    basisMatrix_.resize(
        static_cast<Eigen::Index>(numRows_),
        static_cast<Eigen::Index>(numRows_)
    );

    std::vector<Triplet> basisEntries;
    basisEntries.reserve(numRows_);

    std::size_t basisColumn = 0;

    for (const auto& [row, column] : initialBasisColumns)
    {
        (void)row;

        for (
            typename SparseMatrix::InnerIterator it(
                *constraintMatrix_,
                static_cast<Eigen::Index>(column)
            );
            it;
            ++it
            )
        {
            basisEntries.emplace_back(
                it.row(),
                static_cast<Eigen::Index>(basisColumn),
                it.value()
            );
        }

        ++basisColumn;
    }

    basisMatrix_.setFromTriplets(
        basisEntries.begin(),
        basisEntries.end()
    );

    basisColumnIndices_.resize(numRows_);

    std::size_t i = 0;

    for (const auto& [row, column] : initialBasisColumns)
    {
        (void)row;
        basisColumnIndices_[i] = column;
        ++i;
    }
}

template <typename Scalar>
void BFE<Scalar>::pivot(
    std::size_t enteringColIndex,
    std::size_t leavingColIndex
)
{
    basisMatrix_.col(
        static_cast<Eigen::Index>(leavingColIndex)
    ) =
        constraintMatrix_->col(
            static_cast<Eigen::Index>(enteringColIndex)
        );

    basisColumnIndices_[leavingColIndex] = enteringColIndex;
}

template <typename Scalar>
void BFE<Scalar>::factorize()
{
    basisLU_.compute(basisMatrix_);
}


//template<typename Scalar>
//Eigen::SparseLU<Eigen::SparseMatrix<double>>& BFE<Scalar>::getBasisLU() {
//	return basisLU_;
//}