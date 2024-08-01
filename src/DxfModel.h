#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct DxfCoord
{
    double x = 0.;
    double y = 0.;
    double z = 0.;
};

struct DxfLayer
{
    std::string  name;
    std::int64_t color = 0;
};

struct DxfEntity
{
    std::string                 layer;
    std::optional<std::int64_t> color;
    std::optional<std::string>  peURL;
};

struct DxfLine : DxfEntity
{
    DxfCoord p1;
    DxfCoord p2;
};

struct DxfArc : DxfEntity
{
    DxfCoord center;
    double   radius = 0;
    double   theta1 = 0;
    double   theta2 = 0;
};

struct DxfPolyline : DxfEntity
{
    std::vector<DxfCoord> coords;
    bool                  closed = false;
};

struct DxfModel
{
    std::vector<DxfLayer>    layers;
    std::vector<DxfLine>     lines;
    std::vector<DxfArc>      arcs;
    std::vector<DxfPolyline> polylines;
};