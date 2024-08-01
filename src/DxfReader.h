#pragma once

#include <filesystem>

struct DxfModel;

DxfModel readDxf(const std::filesystem::path& filePath);