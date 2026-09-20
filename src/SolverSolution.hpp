#pragma once
#include <vector>

#include "ESolverAlgorithm.hpp"
#include "types.hpp"

template <typename T>
struct SolverSolution {
	SolverAlgorithm algo;
	T objectiveValue{};
	ProblemType type;

	std::vector<T> variables;
};