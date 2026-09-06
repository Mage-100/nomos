#include <stdexcept>
#include <utility>

#include "LPSolverBuilder.hpp"


LPSolverBuilder& LPSolverBuilder::setConstraintCount(std::size_t count)
{
    constraintCount_ = count;
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setVariableCount(std::size_t count)
{
    variableCount_ = count;
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setSlackVariableCount(std::size_t count)
{
    slackVariableCount_ = count;
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setArtificialVariableCount(
    std::size_t count)
{
    artificialVariableCount_ = count;
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setObjective(
    Eigen::SparseVector<double> objective)
{
    objective_ = std::move(objective);
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setRightHandSide(
    Eigen::SparseVector<double> rhs)
{
    if (rhs.size() != static_cast<Eigen::Index>(constraintCount_)) {
        throw std::invalid_argument(
            "Right-hand side size must equal constraint count."
        );
    }

    rhs_ = std::move(rhs);
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setConstraintMatrix(
    std::vector<Eigen::Triplet<double>> entries)
{
    constraintMatrix_ = std::move(entries);
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setSlackEntries(
    std::vector<Eigen::Triplet<double>> entries)
{
    slackEntries_ = std::move(entries);
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setArtificialEntries(
    std::vector<Eigen::Triplet<double>> entries)
{
    artificialEntries_ = std::move(entries);
    return *this;
}

LPSolverBuilder& LPSolverBuilder::setInitialBasis(
    std::vector<std::pair<std::size_t, std::size_t>> basis)
{
    initialBasis_ = std::move(basis);
    return *this;
}

LPSolver<double> LPSolverBuilder::build()
{
    return {
        constraintCount_,
        variableCount_,
        slackVariableCount_,
        artificialVariableCount_,
        std::move(objective_),
        std::move(rhs_),
        std::move(constraintMatrix_),
        std::move(slackEntries_),
        std::move(artificialEntries_),
        std::move(initialBasis_),
        SolverAlgorithm::LP_REVISED_SIMPLEX
    };
}

