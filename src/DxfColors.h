#pragma once

#include <array>
#include <cstdint>
#include <optional>

std::array<std::uint8_t, 3> dxfColorToRGB(std::uint8_t dxfColor);
std::optional<std::uint8_t> dxfColorFromRGB(const std::array<std::uint8_t, 3>& rgbColor);
