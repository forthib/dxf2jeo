#include "JeoReader.h"

#include "JeoModel.h"
#include <fmt/format.h>
#include <fstream>
#include <jsoncons/json.hpp>

namespace {
    // clang-format off
    template<typename T> struct Type{};
    // clang-format on

    std::optional<std::uint64_t> toOptIndex(std::int64_t value)
    {
        if (value < 0)
            return std::nullopt;
        else
            return static_cast<std::uint64_t>(value);
    }

    auto fromArray(Type<JeoColor>, const std::array<std::uint8_t, 3>& colorArray) { return JeoColor{colorArray[0], colorArray[1], colorArray[2]}; }
    auto fromArray(Type<JeoPoint>, const std::array<double, 3>& pointArray) { return JeoPoint{pointArray[0], pointArray[1], pointArray[2]}; }

    auto fromArray(Type<JeoLine>, const std::array<std::int64_t, 4>& lineArray)
    {
        auto line            = JeoLine{};
        line.firstPointIndex = static_cast<std::uint64_t>(lineArray[0]);
        line.lastPointIndex  = static_cast<std::uint64_t>(lineArray[1]);
        line.colorIndex      = toOptIndex(lineArray[2]);
        line.tagIndex        = toOptIndex(lineArray[3]);
        return line;
    }

    auto fromArray(Type<JeoArc>, const std::array<std::int64_t, 6>& arcArray)
    {
        auto arc            = JeoArc{};
        arc.centerIndex     = static_cast<std::uint64_t>(arcArray[0]);
        arc.firstPointIndex = static_cast<std::uint64_t>(arcArray[1]);
        arc.lastPointIndex  = static_cast<std::uint64_t>(arcArray[2]);
        arc.direct          = arcArray[3] == 1;
        arc.colorIndex      = toOptIndex(arcArray[4]);
        arc.tagIndex        = toOptIndex(arcArray[5]);
        return arc;
    }

    auto fromVector(Type<JeoPolyline>, const std::vector<std::int64_t>& polylineArray)
    {
        if (polylineArray.size() < 2)
            throw std::runtime_error{"invalid format for polyline (size < 2)"};

        const auto nPoints = polylineArray.size() - 2;

        auto polyline = JeoPolyline{};
        polyline.pointIndexes.resize(nPoints);
        for (std::uint64_t i = 0; i < nPoints; ++i)
            polyline.pointIndexes[i] = static_cast<std::uint64_t>(polylineArray[i]);
        polyline.colorIndex = toOptIndex(polylineArray[nPoints]);
        polyline.tagIndex   = toOptIndex(polylineArray[nPoints + 1]);
        return polyline;
    }

    template<typename T, typename ValueArray> auto fromArrays(const std::vector<ValueArray>& valueArrays)
    {
        auto values = std::vector<T>(valueArrays.size());
        std::transform(valueArrays.begin(), valueArrays.end(), values.begin(), [&](const auto& valueArray) { return fromArray(Type<T>{}, valueArray); });
        return values;
    }

    template<typename T, typename ValueVector> auto fromVectors(const std::vector<ValueVector>& valueVectors)
    {
        auto values = std::vector<T>(valueVectors.size());
        std::transform(valueVectors.begin(), valueVectors.end(), values.begin(), [&](const auto& valueVector) { return fromVector(Type<T>{}, valueVector); });
        return values;
    }
}

JeoModel readJeo(const std::filesystem::path& filePath)
{
    auto in = std::ifstream{filePath};
    if (!in.is_open())
        throw std::runtime_error{fmt::format("unable to read file {}", filePath.string())};

    const auto json = jsoncons::ojson::parse(in);

    const auto jeoVersionMajor = json["version"]["major"].as<std::uint64_t>();
    const auto jeoVersionMinor = json["version"]["minor"].as<std::uint64_t>();
    if (jeoVersionMajor != 1 || jeoVersionMinor != 0)
        throw std::runtime_error{fmt::format("unsupported version number: {}.{}", jeoVersionMajor, jeoVersionMinor)};

    auto jeoModel      = JeoModel{};
    jeoModel.colors    = fromArrays<JeoColor>(json["colors"].as<std::vector<std::array<std::uint8_t, 3>>>());
    jeoModel.tags      = json["tags"].as<std::vector<std::string>>();
    jeoModel.points    = fromArrays<JeoPoint>(json["points"].as<std::vector<std::array<double, 3>>>());
    jeoModel.lines     = fromArrays<JeoLine>(json["lines"].as<std::vector<std::array<std::int64_t, 4>>>());
    jeoModel.arcs      = fromArrays<JeoArc>(json["arcs"].as<std::vector<std::array<std::int64_t, 6>>>());
    jeoModel.polylines = fromVectors<JeoPolyline>(json["polylines"].as<std::vector<std::vector<std::int64_t>>>());

    return jeoModel;
}