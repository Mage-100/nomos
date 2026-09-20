#include <iostream>
#include <filesystem>
#include <print>
#include <string_view>
#include <thread>
#include <Eigen/Sparse>

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include "PrintUI.hpp"

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

        PrintUI<double>::PrintLPStats(
            constraintCount, 
            variableCount, 
            rhs.size(), 
            slackCount, 
            artificialCount);

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

        auto screen = ftxui::ScreenInteractive::TerminalOutput();
        auto exitLoop = screen.ExitLoopClosure();

        std::thread solverThread([&]() {
			solver.solve();
            screen.Post(ftxui::Event::Custom);
        });

        std::thread progressThread([&] {
            while (!solver.isSolveDone()) {
                screen.Post(Event::Custom);
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100)
                );
            }
            screen.Post(Event::Custom);
            exitLoop();
		});

        auto renderer = ftxui::Renderer([&]() {
            const double progress = solver.getSolverProgress();
            auto progressBar = ftxui::gauge(progress)
                | ftxui::color(ftxui::Color::Green)
                | ftxui::bgcolor(ftxui::Color::GrayDark)
                | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 50);

            return ftxui::vbox({
                ftxui::text("Solving -") | ftxui::bold,
                hbox({
                    progressBar,
                    ftxui::separatorEmpty(),
                    ftxui::text(std::format("{:.1f}%", progress * 100.0))
				})
                });
		});

        screen.Loop(renderer);


        if (solverThread.joinable())
            solverThread.join();
        if (progressThread.joinable())
            progressThread.join();
        
		auto& solution = solver.getSolution();

		PrintUI<double>::PrintTable(PrintUI<double>::PrepareTableData(solution));
		PrintUI<double>::PrintObjectiveValue(solution);

    }
	 catch (const std::exception& error) {
	    std::println(stderr, "Solver error: {}", error.what());
	    return 1;
     }


    return 0;
}
