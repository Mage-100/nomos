#include <iostream>
#include <filesystem>
#include <print>

#include <Eigen/Sparse>

#include "MPSParser.hpp"
#include "BuildSolver.hpp"

int main(int argc, char* argv[]) {
	try {
    const auto filePath = argc > 1
        ? std::filesystem::path(argv[1])
        : std::filesystem::path(DATASET_DIR) / "afiro.mps";

    MPSParser parser(filePath);
    auto c           = parser.takeObjectiveRow();
    auto b           = parser.takeRHSColumn();
    auto entries     = parser.takeColumnEntries();
    auto slacks      = parser.takeSlackEntries();
    auto artificials = parser.takeArtificialEntries();

    auto rowCount        = parser.getRowCount();
    auto columnCount     = parser.getColumnCount();
    auto slackCount      = parser.getSlacksCount();
    auto artificialCount = parser.getArtificialsCount();

    auto initialBasisEntries = parser.takeInitialBasisEntries();

    std::println("Row Count: {}", rowCount);
    std::println("Column Count: {}", columnCount);
    std::println("RHS Count: {}", b.rows());

    std::println("Slack/surplus columns: {}", slackCount);
    std::println("Artificial columns: {}", artificialCount);
    std::println("Initial basis entries: {}", initialBasisEntries.size());

    BuildSolver builder = BuildSolver()
        .setRows(rowCount)
        .setColumns(columnCount)
        .setSlackCount(slackCount)
        .setArtificialCount(artificialCount)
        .setObjectiveRow(std::move(c))
        .setRHS(std::move(b))
        .setColumnEntries(std::move(entries))
        .setSlacks(std::move(slacks))
        .setArtificials(std::move(artificials))
        .setInitialBasisColumn(std::move(initialBasisEntries));

    auto solver = builder.build();
    solver.solve();
	}
	catch (const std::exception& error) {
		std::println(stderr, "Solver error: {}", error.what());
		return 1;
	}


    return 0;
}
