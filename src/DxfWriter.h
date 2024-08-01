#pragma once

#include <filesystem>

struct DxfModel;

void writeDxf(const DxfModel& model, const std::filesystem::path& filePath);