#include <iostream>
#include <filesystem>
#include <print>
#include <string_view>
#include <Eigen/Sparse>

#include "MPSParser.hpp"
#include "LPSolverBuilder.hpp"
#include "Utility.hpp"

int main(int argc, char* argv[]) {
	try {
        const auto filePath = argc > 1
            ? std::filesystem::path(argv[1])
            : datasetTypeToPath(DatasetType::Afiro);

        MPSParser parser(filePath);
        auto objective =         parser.takeObjectiveRow();
        auto rhs =               parser.takeRHSColumn();
        auto constraintEntries = parser.takeColumnEntries();
        auto slackEntries =      parser.takeSlackEntries();
        auto artificialEntries = parser.takeArtificialEntries();

        auto constraintCount =   parser.getRowCount();
        auto variableCount =     parser.getColumnCount();
        auto slackCount =        parser.getSlacksCount();
        auto artificialCount =   parser.getArtificialsCount();



        auto initialBasisEntries = parser.takeInitialBasisEntries();
        std::println("Constraints: {}", constraintCount);
        std::println("Variables: {}", variableCount);
        std::println("RHS entries: {}", rhs.rows());

        std::println("Slack/surplus variables: {}", slackCount);
        std::println("Artificial variables: {}", artificialCount);
        std::println("Initial basis entries: {}", initialBasisEntries.size());

        LPSolverBuilder builder;
        builder
            .setConstraintCount(constraintCount)
            .setVariableCount(variableCount)
            .setSlackVariableCount(slackCount)
            .setArtificialVariableCount(artificialCount)
            .setObjective(std::move(objective))
            .setRightHandSide(std::move(rhs))
            .setConstraintMatrix(std::move(constraintEntries))
            .setSlackEntries(std::move(slackEntries))
            .setArtificialEntries(std::move(artificialEntries))
            .setInitialBasis(std::move(initialBasisEntries));


        auto solver = builder.build();
        solver.solve();
	 }
	 catch (const std::exception& error) {
	    std::println(stderr, "Solver error: {}", error.what());
	    return 1;
     }


    return 0;
}
