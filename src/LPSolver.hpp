#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include <Eigen/Sparse>

#include "ESolverAlgorithm.hpp"
#include "BasisFactorizationEngine.hpp"

template <typename T>
class LPSolver {
    using SparseVector = Eigen::SparseVector<T>;
    using SparseMatrix = Eigen::SparseMatrix<T>;
    using Triplet = Eigen::Triplet<T>;

public:
    LPSolver(
        std::size_t constraintCount,
        std::size_t decisionVariableCount,
        std::size_t slackVariableCount,
        std::size_t artificialVariableCount,
        SparseVector objectiveCoefficients,
        SparseVector rhs,
        std::vector<Triplet> decisionVariableEntries,
        std::vector<Triplet> slackVariableEntries,
        std::vector<Triplet> artificialVariableEntries,
        std::vector<std::pair<std::size_t, std::size_t>> initialBasisColumns,
        SolverAlgorithm algorithm
    );

    void solve();

private:
    void buildConstraintCoefficientMatrix();
    void removeArtificialBasics();

    void solveSimplex(T& objectiveValue, const SparseVector& objectiveCoefficients, std::size_t enteringColumnCount, bool printSolution);
    T extractSolution(const SparseVector& objectiveCoefficients, bool printSolution);

    std::size_t totalColumnCount() const noexcept;

    std::size_t constraintCount_{ 0 };
    std::size_t decisionVariableCount_{ 0 };

    std::size_t slackVariableCount_{ 0 };
    std::size_t artificialVariableCount_{ 0 };

    SolverAlgorithm algorithm_ =
        SolverAlgorithm::LP_REVISED_SIMPLEX;

    // Objective coefficients.
    SparseVector objectiveCoefficients_;

    // Right-hand-side values.
    SparseVector rhs_;

    // Decision-variable coefficients.
    std::vector<Triplet> decisionVariableEntries_;

    // Slack/surplus-variable coefficients.
    std::vector<Triplet> slackVariableEntries_;

    // Artificial-variable coefficients.
    std::vector<Triplet> artificialVariableEntries_;

    // Initial basis matrix column indices.
    std::vector<std::pair<std::size_t, std::size_t>>
        initialBasisColumns_;

    // Constraint coefficient matrix.
    SparseMatrix constraintMatrix_;

    // Basis factorization engine.
    BFE<T> basisFactorization_;
};

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <print>
#include <stdexcept>

template <typename T>
LPSolver<T>::LPSolver(
    std::size_t constraintCount,
    std::size_t decisionVariableCount,
    std::size_t slackVariableCount,
    std::size_t artificialVariableCount,
    SparseVector objectiveCoefficients,
    SparseVector rhs,
    std::vector<Triplet> decisionVariableEntries,
    std::vector<Triplet> slackVariableEntries,
    std::vector<Triplet> artificialVariableEntries,
    std::vector<std::pair<std::size_t, std::size_t>> initialBasisColumns,
    SolverAlgorithm algorithm
)
    : constraintCount_(constraintCount),
    decisionVariableCount_(decisionVariableCount),
    slackVariableCount_(slackVariableCount),
    artificialVariableCount_(artificialVariableCount),
    objectiveCoefficients_(std::move(objectiveCoefficients)),
    rhs_(std::move(rhs)),
    decisionVariableEntries_(std::move(decisionVariableEntries)),
    slackVariableEntries_(std::move(slackVariableEntries)),
    artificialVariableEntries_(std::move(artificialVariableEntries)),
    initialBasisColumns_(std::move(initialBasisColumns)),
    algorithm_(algorithm)
{
    objectiveCoefficients_.conservativeResize(
        static_cast<Eigen::Index>(totalColumnCount())
    );

    buildConstraintCoefficientMatrix();

    //assert(initialBasisColumns_.size() == constraintCount_);

    basisFactorization_.initialize(
        constraintMatrix_,
        initialBasisColumns_
    );
}

    template <typename T>
    void LPSolver<T>::solve()
    {
        constexpr T tolerance = static_cast<T>(1e-8);

        const std::size_t nonArtificialColumnCount =
            decisionVariableCount_ + slackVariableCount_;

        // Phase I minimizes the sum of artificial variables.
        if (artificialVariableCount_ > 0)
        {
            SparseVector phaseOneObjective(
                static_cast<Eigen::Index>(totalColumnCount())
            );

            for (std::size_t i = nonArtificialColumnCount;
                i < totalColumnCount();
                ++i)
            {
                phaseOneObjective.insert(
                    static_cast<Eigen::Index>(i)
                ) = static_cast<T>(1);
            }

            T phaseOneObjectiveValue{};

            solveSimplex(
                phaseOneObjectiveValue,
                phaseOneObjective,
                totalColumnCount(),
                false
            );

            if (phaseOneObjectiveValue > tolerance)
            {
                throw std::runtime_error(
                    "Problem is infeasible."
                );
            }

            removeArtificialBasics();
        }

        // Phase II uses the original objective coefficients.
        T objectiveValue{};

        solveSimplex(
            objectiveValue,
            objectiveCoefficients_,
            nonArtificialColumnCount,
            true
        );
    }

template <typename T>
void LPSolver<T>::buildConstraintCoefficientMatrix()
{
    // Allocate the constraint matrix with all variable columns.
    constraintMatrix_.resize(
        static_cast<Eigen::Index>(constraintCount_),
        static_cast<Eigen::Index>(totalColumnCount())
    );

    std::vector<Triplet> matrixEntries;

    // Reserve space for all non-zero entries.
    matrixEntries.reserve(
        decisionVariableEntries_.size() +
        slackVariableEntries_.size() +
        artificialVariableEntries_.size()
    );

    // Add decision-variable coefficients.
    matrixEntries.insert(
        matrixEntries.end(),
        decisionVariableEntries_.begin(),
        decisionVariableEntries_.end()
    );

    // Add slack/surplus-variable coefficients.
    matrixEntries.insert(
        matrixEntries.end(),
        slackVariableEntries_.begin(),
        slackVariableEntries_.end()
    );

    // Add artificial-variable coefficients.
    matrixEntries.insert(
        matrixEntries.end(),
        artificialVariableEntries_.begin(),
        artificialVariableEntries_.end()
    );

    // Construct the sparse constraint matrix.
    constraintMatrix_.setFromTriplets(
        matrixEntries.begin(),
        matrixEntries.end()
    );
}

template <typename T>
std::size_t LPSolver<T>::totalColumnCount() const noexcept
{
    // Return the total number of variables.
    return decisionVariableCount_
        + slackVariableCount_
        + artificialVariableCount_;
}

template <typename T>
void LPSolver<T>::removeArtificialBasics()
{
    constexpr T tolerance = static_cast<T>(1e-9);

    const std::size_t nonArtificialColumnCount =
        decisionVariableCount_ + slackVariableCount_;

    for (std::size_t basisPosition = 0;
        basisPosition < constraintCount_;
        ++basisPosition)
    {
        auto& basisLU = basisFactorization_.getBasisLU();
        const auto& basisIndices =
            basisFactorization_.getBasisColumnIndices();

        // Skip the basis variable if it is not artificial.
        if (basisIndices[basisPosition] <
            nonArtificialColumnCount)
        {
            continue;
        }

        // Compute the current basic solution.
        const Eigen::VectorX<T> basicSolution =
            basisLU.solve(Eigen::VectorX<T>(rhs_));

        // An artificial basic variable must be zero after Phase I.
        if (std::abs(
            basicSolution(
                static_cast<Eigen::Index>(basisPosition)
            )
        ) > tolerance)
        {
            throw std::runtime_error(
                "Problem is infeasible: an artificial variable "
                "remains positive after Phase I."
            );
        }

        std::size_t enteringColumn =
            std::numeric_limits<std::size_t>::max();

        for (std::size_t candidate = 0;
            candidate < nonArtificialColumnCount;
            ++candidate)
        {
            bool isBasic = false;

            // Check whether the candidate is already basic.
            for (const std::size_t basisColumn : basisIndices)
            {
                if (basisColumn == candidate)
                {
                    isBasic = true;
                    break;
                }
            }

            if (isBasic)
            {
                continue;
            }

            // Compute the candidate direction.
            const Eigen::VectorX<T> direction =
                basisLU.solve(
                    Eigen::VectorX<T>(
                        constraintMatrix_.col(
                            static_cast<Eigen::Index>(candidate)
                        )
                    )
                );

            // Select a column that can replace the artificial variable.
            if (std::abs(
                direction(
                    static_cast<Eigen::Index>(basisPosition)
                )
            ) > tolerance)
            {
                enteringColumn = candidate;
                break;
            }
        }

        // No valid entering column means the constraint is redundant.
        if (enteringColumn ==
            std::numeric_limits<std::size_t>::max())
        {
            throw std::runtime_error(
                "Phase I found a redundant constraint. "
                "Removing redundant rows is not implemented yet."
            );
        }

        // Pivot the artificial variable out of the basis.
        basisFactorization_.pivot(
            enteringColumn,
            basisPosition
        );

        // Refactorize the updated basis.
        basisFactorization_.factorize();
    }
}

template <typename T>
void LPSolver<T>::solveSimplex(
    T& objectiveValue,
    const SparseVector& objectiveCoefficients,
    std::size_t enteringColumnCount,
    bool printSolution
)
{
    constexpr T tolerance = static_cast<T>(1e-9);
    constexpr std::size_t maximumIterations = 100'000;

    std::size_t iterations = 0;

    while (true)
    {
        auto& basisLU = basisFactorization_.getBasisLU();
        const auto& basisIndices =
            basisFactorization_.getBasisColumnIndices();

        // Ensure the current basis factorization is valid.
        if (basisLU.info() != Eigen::Success)
        {
            throw std::runtime_error(
                "Basis factorization failed."
            );
        }

        // Compute x_B = B^-1 b.
        const Eigen::VectorX<T> basicSolution =
            basisLU.solve(Eigen::VectorX<T>(rhs_));

        // Build c_B from the current basic variables.
        Eigen::VectorX<T> basicCosts(
            static_cast<Eigen::Index>(constraintCount_)
        );

        for (std::size_t i = 0;
            i < constraintCount_;
            ++i)
        {
            basicCosts(
                static_cast<Eigen::Index>(i)
            ) =
                objectiveCoefficients.coeff(
                    static_cast<Eigen::Index>(
                        basisIndices[i]
                        )
                );
        }

        // Compute lambda from B^T lambda = c_B.
        const Eigen::VectorX<T> simplexMultipliers =
            basisLU.transpose().solve(basicCosts);

        std::size_t enteringColumn =
            std::numeric_limits<std::size_t>::max();

        for (std::size_t candidate = 0;
            candidate < enteringColumnCount;
            ++candidate)
        {
            bool isBasic = false;

            // Skip columns that are already basic.
            for (const std::size_t basisColumn : basisIndices)
            {
                if (basisColumn == candidate)
                {
                    isBasic = true;
                    break;
                }
            }

            if (isBasic)
            {
                continue;
            }

            // Compute the reduced cost.
            const T reducedCost =
                objectiveCoefficients.coeff(
                    static_cast<Eigen::Index>(candidate)
                )
                -
                simplexMultipliers.dot(
                    Eigen::VectorX<T>(
                        constraintMatrix_.col(
                            static_cast<Eigen::Index>(candidate)
                        )
                    )
                );

            // Select a negative reduced cost for minimization.
            if (reducedCost < -tolerance)
            {
                enteringColumn = candidate;
                break;
            }
        }

        // No negative reduced cost means the solution is optimal.
        if (enteringColumn ==
            std::numeric_limits<std::size_t>::max())
        {
            break;
        }

        // Compute the direction d = B^-1 A_entering.
        const Eigen::VectorX<T> direction =
            basisLU.solve(
                Eigen::VectorX<T>(
                    constraintMatrix_.col(
                        static_cast<Eigen::Index>(
                            enteringColumn
                            )
                    )
                )
            );

        std::size_t leavingColumn =
            std::numeric_limits<std::size_t>::max();

        T minimumRatio =
            std::numeric_limits<T>::infinity();

        for (std::size_t i = 0;
            i < constraintCount_;
            ++i)
        {
            const T directionValue =
                direction(
                    static_cast<Eigen::Index>(i)
                );

            // Only positive direction components participate in the ratio test.
            if (directionValue > tolerance)
            {
                const T ratio =
                    basicSolution(
                        static_cast<Eigen::Index>(i)
                    ) / directionValue;

                if (ratio < minimumRatio)
                {
                    minimumRatio = ratio;
                    leavingColumn = i;
                }
            }
        }

        // No leaving variable means the problem is unbounded.
        if (leavingColumn ==
            std::numeric_limits<std::size_t>::max())
        {
            throw std::runtime_error(
                "Problem is unbounded."
            );
        }

        // Replace the leaving basis column.
        basisFactorization_.pivot(
            enteringColumn,
            leavingColumn
        );

        // Refactorize the updated basis.
        basisFactorization_.factorize();

        ++iterations;

        // Print progress.
        if (iterations % 100 == 0)
        {
            const double percentage =
                100.0 * static_cast<double>(iterations) /
                static_cast<double>(maximumIterations);

            std::print(
                "\rProgress: {:6.2f}%",
                percentage
            );
        }

        // Stop if the iteration limit is reached.
        if (iterations >= maximumIterations)
        {
            std::println();

            throw std::runtime_error(
                "Simplex iteration limit reached."
            );
        }

    }

    std::println("Iterations took: {}", iterations);

    // Extract the final solution.
    objectiveValue = extractSolution(objectiveCoefficients, printSolution);
}

template<typename T>
inline T LPSolver<T>::extractSolution(const SparseVector& objectiveCoefficients, bool printSolution)
{

    const auto& basisColumnIndices = basisFactorization_.getBasisColumnIndices();

    auto& basisLU = basisFactorization_.getBasisLU();

    const Eigen::VectorX<T> basicSolution = basisLU.solve(Eigen::VectorX<T>(rhs_));

    Eigen::VectorX<T> solution = Eigen::VectorX<T>::Zero(static_cast<Eigen::Index>(totalColumnCount()));

    for (std::size_t i = 0; i < constraintCount_; ++i)
    {
        solution(
            static_cast<Eigen::Index>(
                basisColumnIndices[i]
                )
        ) = basicSolution(
            static_cast<Eigen::Index>(i)
        );
    }

    const T objectiveValue =
        objectiveCoefficients.dot(solution);

    if (printSolution)
    {
        std::println("\nOptimal Solution:");

        for (std::size_t i = 0;
            i < decisionVariableCount_;
            ++i)
        {
            std::println(
                " x[{}] = {:.6f}",
                i + 1,
                solution(
                    static_cast<Eigen::Index>(i)
                )
            );
        }

        std::println(
            "\nObjective value: {:.15f}",
            objectiveValue
        );
    }

    return objectiveValue;
}