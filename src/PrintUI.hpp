#pragma once
#include <vector>
#include <string>
#include <print>
#include <iomanip>

#include <ftxui/screen/screen.hpp>

#include "SolverSolution.hpp"

using namespace ftxui;

template <typename T>
class PrintUI {
public:
	static void PrintTable(std::vector<std::vector<std::string>> data);
	static std::vector<std::vector<std::string>> PrepareTableData(const SolverSolution<T>& solution);
	static void PrintObjectiveValue(const SolverSolution<T>& solution);

	/*
		Prints LP statistics
		@param constrainst Constraint Count
		@param variableCount Variable Count
		@param rhsCount Right Hand Side Values Count
		@param slacks No. of slacks and surpluses added
		@param artificials No. of artificial variables added
	*/
	static void PrintLPStats(
		const std::size_t constraints,
		const std::size_t variableCount,
		const std::size_t rhsCount,
		const std::size_t slacks,
		const std::size_t artificials);
};

template <typename T>
inline void PrintUI<T>::PrintTable(std::vector<std::vector<std::string>> data)
{
	auto table = Table(data);

	// Style — apply in this exact order, outer to inner
	table.SelectAll().Border();
	table.SelectRow(0).SeparatorVertical(LIGHT);
	table.SelectRow(0).Border(LIGHT);
	table.SelectRow(0).Decorate(bold);

	auto content = table.SelectRows(1, -1);
	// Alternate in between 3 colors.
	content.DecorateCellsAlternateRow(color(Color::BlueLight), 3, 0);
	content.DecorateCellsAlternateRow(color(Color::Cyan), 3, 1);
	content.DecorateCellsAlternateRow(color(Color::White), 3, 2);

	// Render
	auto doc = table.Render();
	auto screen = Screen::Create(
		Dimension::Fit(doc),
		Dimension::Fixed(data.size() + 2)
	);
	Render(screen, doc);
	screen.Print();
	std::cout << std::endl;
}

template <typename T>
inline std::vector<std::vector<std::string>> 
PrintUI<T>::PrepareTableData(const SolverSolution<T>& solution)
{
	std::vector<std::vector<std::string>> data;
	data.push_back({ "Variable", "Value" });

	for (std::size_t i = 0; i < solution.variables.size(); ++i) {
		data.push_back({
			std::format("x[{}]", i + 1),
			std::format("{:.6f}", solution.variables[i])
			});
	}

	return data;
}
template <typename T>
inline void PrintUI<T>::PrintObjectiveValue(const SolverSolution<T>& solution) 
{
	auto document = hbox({
		text("Objective Value: "),
		text(std::format("{:.10E}", solution.objectiveValue)) | bold | color(Color::Green)
		});

	auto screen = Screen::Create(Dimension::Fit(document), Dimension::Fixed(2));
	Render(screen, document);
	screen.Print();
	std::cout << std::endl;
}

template <typename T>
inline void PrintUI<T>::PrintLPStats(
	const std::size_t constraints,
	const std::size_t variableCount,
	const std::size_t rhsCount,
	const std::size_t slacks,
	const std::size_t artificials)
{

	auto stat = [](std::string name, int value) {
		return hbox({
			text(name),
			filler(),
			text(std::to_string(value)) | bold,
			});
		};

	auto document = window(
		text("Problem Statistics") | bold | center,
		vbox({
			stat("Constraints", constraints),
			stat("Variables", variableCount),
			stat("RHS Values", rhsCount),
			stat("Slacks/Surpluses", slacks),
			stat("Artificials", artificials),
			})
			) | size(WIDTH, GREATER_THAN, 30);

	auto screen = Screen::Create(Dimension::Fit(document), Dimension::Fit(document));
	Render(screen, document);
	screen.Print();
	std::cout << std::endl;
	
}
