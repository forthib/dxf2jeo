#include "Dxf2Jeo.h"

#include "DxfColors.h"
#include "DxfModel.h"
#include "JeoModel.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string_view>

namespace {
    constexpr auto DISTANCE_TOLERANCE = 1e-3;

    template<typename T> std::uint64_t add(std::vector<T>& values, T value)
    {
        const auto index = static_cast<std::uint64_t>(values.size());
        values.push_back(std::move(value));
        return index;
    }

    template<typename T> std::uint64_t addUnique(std::vector<T>& values, T value)
    {
        const auto it = std::find(values.begin(), values.end(), value);
        if (it != values.end())
            return static_cast<std::uint64_t>(std::distance(values.begin(), it));
        else
            return add(values, std::move(value));
    }

    DxfCoord evaluate(const DxfArc& arc, double u)
    {
        const auto theta = arc.theta1 + u * (arc.theta2 - arc.theta1);
        const auto x     = arc.center.x + arc.radius * std::cos(theta);
        const auto y     = arc.center.y + arc.radius * std::sin(theta);
        const auto z     = arc.center.z;
        return {x, y, z};
    }

    std::uint64_t addPoint(JeoModel& jeoModel, const DxfCoord& dxfCoord)
    {
        const auto it = std::find_if(jeoModel.points.begin(), jeoModel.points.end(), [&](const JeoPoint& jeoPoint) {
            const double dx = jeoPoint.x - dxfCoord.x;
            const double dy = jeoPoint.y - dxfCoord.y;
            const double dz = jeoPoint.z - dxfCoord.z;
            return std::sqrt(dx * dx + dy * dy + dz * dz) <= DISTANCE_TOLERANCE;
        });

        if (it != jeoModel.points.end())
            return static_cast<std::uint64_t>(std::distance(jeoModel.points.begin(), it));

        return add(jeoModel.points, {dxfCoord.x, dxfCoord.y, dxfCoord.z});
    }

    std::vector<std::uint64_t> addPoints(JeoModel& jeoModel, const std::vector<DxfCoord>& dxfCoords)
    {
        auto ids = std::vector<std::uint64_t>(dxfCoords.size());
        std::transform(dxfCoords.begin(), dxfCoords.end(), ids.begin(), [&](const DxfCoord& dxfCoord) { return addPoint(jeoModel, dxfCoord); });
        return ids;
    }

    bool isTagChar(char c) { return c == '_' || std::isalnum(static_cast<unsigned char>(c)); }
    bool isTag(std::string_view tag) { return std::all_of(tag.begin(), tag.end(), isTagChar); }

    std::optional<std::uint64_t> addTag(JeoModel& jeoModel, const std::optional<std::string>& tag)
    {
        if (!tag || !isTag(*tag))
            return std::nullopt;
        return addUnique(jeoModel.tags, *tag);
    }

    JeoColor dxf2JeoColor(std::int64_t dxfColor)
    {
        if (dxfColor <= 0 || dxfColor > 255)
            throw std::runtime_error{"unsupported dxf color"};

        const auto [r, g, b] = dxfColorToRGB(static_cast<std::uint8_t>(dxfColor));
        return {r, g, b};
    }

    std::uint64_t addColor(JeoModel& jeoModel, std::int64_t dxfColor)
    {
        const auto jeoColor = dxf2JeoColor(dxfColor);

        const auto it = std::find_if(jeoModel.colors.begin(), jeoModel.colors.end(), [&](const JeoColor& jeoColor2) {
            return jeoColor.r == jeoColor2.r && jeoColor.g == jeoColor2.g && jeoColor.b == jeoColor2.b;
        });

        if (it != jeoModel.colors.end())
            return static_cast<std::uint64_t>(std::distance(jeoModel.colors.begin(), it));

        return add(jeoModel.colors, jeoColor);
    }

    std::optional<std::uint64_t> addColor(JeoModel& jeoModel, std::optional<std::int64_t> dxfColor)
    {
        if (!dxfColor || *dxfColor <= 0 || *dxfColor > 255)
            return std::nullopt;
        return addColor(jeoModel, *dxfColor);
    }

    void setEntity(JeoModel& jeoModel, JeoEntity& jeoEntity, const DxfEntity& dxfEntity)
    {
        jeoEntity.colorIndex = addColor(jeoModel, dxfEntity.color);
        jeoEntity.tagIndex   = addTag(jeoModel, dxfEntity.peURL);
    }

    void addLine(JeoModel& jeoModel, const DxfLine& dxfLine)
    {
        auto jeoLine            = JeoLine{};
        jeoLine.firstPointIndex = addPoint(jeoModel, dxfLine.p1);
        jeoLine.lastPointIndex  = addPoint(jeoModel, dxfLine.p2);
        setEntity(jeoModel, jeoLine, dxfLine);
        jeoModel.lines.push_back(jeoLine);
    }

    void addArc(JeoModel& jeoModel, DxfArc dxfArc)
    {
        auto jeoArc            = JeoArc{};
        jeoArc.centerIndex     = addPoint(jeoModel, dxfArc.center);
        jeoArc.firstPointIndex = addPoint(jeoModel, evaluate(dxfArc, 0.));
        jeoArc.lastPointIndex  = addPoint(jeoModel, evaluate(dxfArc, 1.));
        jeoArc.direct          = dxfArc.theta1 <= dxfArc.theta2;
        setEntity(jeoModel, jeoArc, dxfArc);
        jeoModel.arcs.push_back(jeoArc);
    }

    void addPolyline(JeoModel& jeoModel, const DxfPolyline& dxfPolyline)
    {
        if (dxfPolyline.coords.size() < 2)
            throw std::runtime_error{"unsupported polyline"};

        auto jeoPolyline         = JeoPolyline{};
        jeoPolyline.pointIndexes = addPoints(jeoModel, dxfPolyline.coords);
        if (dxfPolyline.closed)
            jeoPolyline.pointIndexes.push_back(jeoPolyline.pointIndexes.front());
        jeoModel.polylines.push_back(jeoPolyline);
        setEntity(jeoModel, jeoPolyline, dxfPolyline);
        jeoModel.polylines.push_back(jeoPolyline);
    }
}

JeoModel convertToJeo(const DxfModel& dxfModel)
{
    auto jeoModel = JeoModel{};
    for (const auto& line : dxfModel.lines)
        addLine(jeoModel, line);
    for (const auto& arc : dxfModel.arcs)
        addArc(jeoModel, arc);
    for (const auto& polyline : dxfModel.polylines)
        addPolyline(jeoModel, polyline);
    return jeoModel;
}