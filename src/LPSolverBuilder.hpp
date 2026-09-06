#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include <Eigen/Sparse>

#include "LPSolver.hpp"

class LPSolverBuilder {
public:
    LPSolverBuilder() = default;

    // Setter
    LPSolverBuilder& setConstraintCount(std::size_t count);
    LPSolverBuilder& setVariableCount(std::size_t count);

    LPSolverBuilder& setSlackVariableCount(std::size_t count);
    LPSolverBuilder& setArtificialVariableCount(std::size_t count);

    LPSolverBuilder& setObjective(Eigen::SparseVector<double> objective);
    LPSolverBuilder& setRightHandSide(Eigen::SparseVector<double> rhs);

    LPSolverBuilder& setConstraintMatrix(std::vector<Eigen::Triplet<double>> entries);

    LPSolverBuilder& setSlackEntries(std::vector<Eigen::Triplet<double>> entries);
    LPSolverBuilder& setArtificialEntries(std::vector<Eigen::Triplet<double>> entries);
    LPSolverBuilder& setInitialBasis(std::vector<std::pair<std::size_t, std::size_t>> basis);

    // Core
    LPSolver<double> build();

private:
    std::size_t constraintCount_ = 0;
    std::size_t variableCount_ = 0;

    std::size_t slackVariableCount_ = 0;
    std::size_t artificialVariableCount_ = 0;

    Eigen::SparseVector<double> objective_;
    Eigen::SparseVector<double> rhs_;

    std::vector<Eigen::Triplet<double>> constraintMatrix_;
    std::vector<Eigen::Triplet<double>> slackEntries_;
    std::vector<Eigen::Triplet<double>> artificialEntries_;

    std::vector<std::pair<std::size_t, std::size_t>> initialBasis_;
};
