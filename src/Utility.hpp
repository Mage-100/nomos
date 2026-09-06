#include <filesystem>
#include <string_view>
#include "types.hpp"

std::filesystem::path datasetTypeToPath(DatasetType type) {

    switch (type)
    {
    case DatasetType::Afiro: return std::filesystem::path(DATASET_DIR) / "afiro.mps";
    case DatasetType::Agg:   return std::filesystem::path(DATASET_DIR) / "agg.mps";
    case DatasetType::Agg2:  return std::filesystem::path(DATASET_DIR) / "agg2.mps";
    case DatasetType::Agg3:  return std::filesystem::path(DATASET_DIR) / "agg3.mps";
    case DatasetType::Bandm: return std::filesystem::path(DATASET_DIR) / "bandm.mps";
    case DatasetType::Bnl1:  return std::filesystem::path(DATASET_DIR) / "bnl1.mps";
    case DatasetType::Bnl2:  return std::filesystem::path(DATASET_DIR) / "bnl2.mps";
    };
    return {};
}
std::filesystem::path datasetTypeToPath(const char* type)
{
    const std::string_view name(type);
    if (name == "afiro") return datasetTypeToPath(DatasetType::Afiro);
    if (name == "agg")   return datasetTypeToPath(DatasetType::Agg);
    if (name == "agg2")  return datasetTypeToPath(DatasetType::Agg2);
    if (name == "agg3")  return datasetTypeToPath(DatasetType::Agg3);
    if (name == "bandm") return datasetTypeToPath(DatasetType::Bandm);
    if (name == "bnl1")  return datasetTypeToPath(DatasetType::Bnl1);
    if (name == "bnl2")  return datasetTypeToPath(DatasetType::Bnl2);

    return {};
}