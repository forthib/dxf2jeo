#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct JeoColor
{
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

struct JeoPoint
{
    double x = 0.;
    double y = 0.;
    double z = 0.;
};

struct JeoEntity
{
    std::optional<uint64_t> colorIndex;
    std::optional<uint64_t> tagIndex;
};

struct JeoLine : JeoEntity
{
    std::uint64_t firstPointIndex = 0;
    std::uint64_t lastPointIndex  = 0;
};

struct JeoArc : JeoEntity
{
    std::uint64_t centerIndex     = 0;
    std::uint64_t firstPointIndex = 0;
    std::uint64_t lastPointIndex  = 0;
    bool          direct          = true;
};

struct JeoPolyline : JeoEntity
{
    std::vector<std::uint64_t>         pointIndexes;
    std::optional<std::vector<double>> bulges;
    bool                               closed = false;
};

struct JeoModel
{
    std::vector<JeoColor>    colors;
    std::vector<std::string> tags;
    std::vector<JeoPoint>    points;
    std::vector<JeoLine>     lines;
    std::vector<JeoArc>      arcs;
    std::vector<JeoPolyline> polylines;
};