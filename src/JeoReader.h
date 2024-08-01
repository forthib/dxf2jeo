#pragma once

#include <filesystem>

struct JeoModel;

JeoModel readJeo(const std::filesystem::path& filePath);