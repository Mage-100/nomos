#include <iostream>
#include <filesystem>
#include "MPSParser.hpp"
#include <print>
int main() {
    const auto filePath = std::filesystem::path(DATASET_DIR) / "afiro.mps";

    MPSParser parser(filePath);
    auto& entries = parser.getEntries();

    std::println("Column Count: {}", parser.getColumnCount());
    std::println("Row Count: {}", parser.getRowCount());

    return 0;
}
