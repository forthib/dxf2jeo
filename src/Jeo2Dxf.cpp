#include "Jeo2Dxf.h"

#include "DxfColors.h"
#include "DxfModel.h"
#include "JeoModel.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

    static const auto PI = std::atan(1.) * 4;

    double evaluateDistance(const JeoPoint& p1, const JeoPoint& p2)
    { //
        return std::hypot(p2.x - p1.x, p2.y - p1.y);
    }

    double evaluateArcRadius(const JeoModel& jeoModel, const JeoArc& jeoArc)
    {
        const auto jeoCenter = jeoModel.points[jeoArc.centerIndex];
        const auto jeoP1     = jeoModel.points[jeoArc.firstPointIndex];
        const auto jeoP2     = jeoModel.points[jeoArc.lastPointIndex];
        const auto dist1     = evaluateDistance(jeoCenter, jeoP1);
        const auto dist2     = evaluateDistance(jeoCenter, jeoP2);
        return (dist1 + dist2) / 2.;
    }

    double evaluateArcTheta(const JeoModel& jeoModel, const JeoArc& jeoArc, std::uint64_t pointIndex)
    {
        const auto jeoCenter = jeoModel.points[jeoArc.centerIndex];
        const auto jeoPoint  = jeoModel.points[pointIndex];
        return std::atan2(jeoPoint.y - jeoCenter.y, jeoPoint.x - jeoCenter.x);
    }

    std::optional<std::uint8_t> toDxfColor(const JeoModel& jeoModel, std::optional<uint64_t> colorIndex)
    {
        if (!colorIndex)
            return std::nullopt;

        const auto [r, g, b] = jeoModel.colors[*colorIndex];
        return dxfColorFromRGB({r, g, b});
    }

    std::optional<std::string> toDxfPEURL(const JeoModel& jeoModel, std::optional<std::uint64_t> tagIndex)
    {
        if (!tagIndex)
            return std::nullopt;
        return jeoModel.tags[*tagIndex];
    }

    DxfCoord toDxfCoord(const JeoModel& jeoModel, std::uint64_t pointIndex)
    {
        const auto jeoPoint = jeoModel.points[pointIndex];
        return {jeoPoint.x, jeoPoint.y, jeoPoint.z};
    }

    std::vector<DxfCoord> toDxfCoords(const JeoModel& jeoModel, const std::vector<std::uint64_t>& pointIndexes)
    {
        auto dxfCoords = std::vector<DxfCoord>(pointIndexes.size());
        std::transform(pointIndexes.begin(), pointIndexes.end(), dxfCoords.begin(), [&](std::uint64_t pointIndex) { return toDxfCoord(jeoModel, pointIndex); });
        return dxfCoords;
    }

    template<typename DxfEntity> DxfEntity toDxfEntity(const JeoModel& jeoModel, const JeoEntity& jeoEntity)
    {
        auto dxfEntity  = DxfEntity{};
        dxfEntity.color = toDxfColor(jeoModel, jeoEntity.colorIndex);
        dxfEntity.peURL = toDxfPEURL(jeoModel, jeoEntity.tagIndex);
        return dxfEntity;
    }

    DxfLine toDxfLine(const JeoModel& jeoModel, const JeoLine& jeoLine)
    {
        auto dxfLine = toDxfEntity<DxfLine>(jeoModel, jeoLine);
        dxfLine.p1   = toDxfCoord(jeoModel, jeoLine.firstPointIndex);
        dxfLine.p2   = toDxfCoord(jeoModel, jeoLine.lastPointIndex);
        return dxfLine;
    }

    DxfArc toDxfArc(const JeoModel& jeoModel, const JeoArc& jeoArc)
    {
        auto dxfArc   = toDxfEntity<DxfArc>(jeoModel, jeoArc);
        dxfArc.center = toDxfCoord(jeoModel, jeoArc.centerIndex);
        dxfArc.radius = evaluateArcRadius(jeoModel, jeoArc);
        dxfArc.theta1 = evaluateArcTheta(jeoModel, jeoArc, jeoArc.firstPointIndex);
        dxfArc.theta2 = evaluateArcTheta(jeoModel, jeoArc, jeoArc.lastPointIndex);
        if (dxfArc.theta1 >= dxfArc.theta2) {
            std::swap(dxfArc.theta1, dxfArc.theta2);
            dxfArc.theta2 += 2 * PI;
        }
        return dxfArc;
    }

    DxfPolyline toDxfPolyline(const JeoModel& jeoModel, const JeoPolyline& jeoPolyline)
    {
        if (jeoPolyline.pointIndexes.size() < 2)
            throw std::runtime_error{"unsupported polyline"};

        auto dxfPolyline   = toDxfEntity<DxfPolyline>(jeoModel, jeoPolyline);
        dxfPolyline.coords = toDxfCoords(jeoModel, jeoPolyline.pointIndexes);
        dxfPolyline.closed = jeoPolyline.pointIndexes.front() == jeoPolyline.pointIndexes.back();
        return dxfPolyline;
    }
}

DxfModel convertToDxf(const JeoModel& jeoModel)
{
    auto dxfModel = DxfModel{};
    for (const auto& line : jeoModel.lines)
        dxfModel.lines.push_back(toDxfLine(jeoModel, line));
    for (const auto& arc : jeoModel.arcs)
        dxfModel.arcs.push_back(toDxfArc(jeoModel, arc));
    for (const auto& polyline : jeoModel.polylines)
        dxfModel.polylines.push_back(toDxfPolyline(jeoModel, polyline));
    return dxfModel;
}