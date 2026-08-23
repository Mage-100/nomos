#include <iostream>
#include <filesystem>
#include "MPSParser.hpp"

using fs_path = std::filesystem::path;

int main() {
    fs_path filepath = fs_path(DATASET_DIR) / "afiro.mps";

    MPSParser parser(filepath.string());

    std::printf("Column Count: %d\n", parser.getColumnCount());
    std::printf("Row Count: %d\n", parser.getRowCount());

    return 0;
}
