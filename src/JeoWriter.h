#pragma once

#include <filesystem>

struct JeoModel;

void writeJeo(const JeoModel& model, const std::filesystem::path& filePath);