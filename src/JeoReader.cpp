#include "JeoReader.h"

#include "JeoModel.h"
#include <fmt/format.h>
#include <fstream>
#include <jsoncons/json.hpp>

namespace {
    // clang-format off
    template<typename T> struct Type{};
    // clang-format on

    auto fromJson(Type<JeoColor>, const jsoncons::ojson& json)
    {
        const auto colorArray = json.as<std::array<std::uint8_t, 3>>();
        return JeoColor{colorArray[0], colorArray[1], colorArray[2]};
    }

    auto fromJson(Type<JeoPoint>, const jsoncons::ojson& json)
    {
        const auto pointArray = json.as<std::array<double, 3>>();
        return JeoPoint{pointArray[0], pointArray[1], pointArray[2]};
    }

    auto fromJson(Type<JeoEntity>, const jsoncons::ojson& json)
    {
        auto entity = JeoEntity{};
        if (json.contains("color"))
            entity.colorIndex = json["color"].as<uint64_t>();
        if (json.contains("tag"))
            entity.tagIndex = json["tag"].as<uint64_t>();
        return entity;
    }

    auto fromJson(Type<JeoLine>, const jsoncons::ojson& json)
    {
        auto       line         = JeoLine{fromJson(Type<JeoEntity>{}, json)};
        const auto pointIndexes = json["points"].as<std::array<std::uint64_t, 2>>();
        line.firstPointIndex    = pointIndexes[0];
        line.lastPointIndex     = pointIndexes[1];
        return line;
    }

    auto fromJson(Type<JeoArc>, const jsoncons::ojson& json)
    {
        auto       arc          = JeoArc{fromJson(Type<JeoEntity>{}, json)};
        const auto pointIndexes = json["points"].as<std::array<std::uint64_t, 3>>();
        arc.centerIndex         = pointIndexes[0];
        arc.firstPointIndex     = pointIndexes[1];
        arc.lastPointIndex      = pointIndexes[2];
        arc.direct              = json["direct"].as_bool();
        return arc;
    }

    auto fromJson(Type<JeoPolyline>, const jsoncons::ojson& json)
    {
        auto polyline         = JeoPolyline{fromJson(Type<JeoEntity>{}, json)};
        polyline.pointIndexes = json["points"].as<std::vector<std::uint64_t>>();
        if (json.contains("bulges"))
            polyline.bulges = json["bulges"].as<std::vector<double>>();
        polyline.closed = json["closed"].as_bool();

        if (polyline.bulges && polyline.bulges->size() != polyline.pointIndexes.size())
            throw std::runtime_error{"size of points and bulges must be equal"};

        return polyline;
    }

    template<typename T> auto fromJson(Type<std::vector<T>>, const jsoncons::ojson& json)
    {
        if (!json.is_array())
            throw std::runtime_error{"json element must be an array"};

        auto elements = std::vector<T>{};
        elements.reserve(json.size());
        for (uint64_t i = 0, n = json.size(); i < n; ++i)
            elements.push_back(fromJson(Type<T>{}, json[i]));
        return elements;
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
    if (jeoVersionMajor < 2)
        throw std::runtime_error{"jeo file with version < 2 are no longer supported"};
    if (jeoVersionMajor != 2 || jeoVersionMinor != 0)
        throw std::runtime_error{fmt::format("unsupported version number: {}.{}", jeoVersionMajor, jeoVersionMinor)};

    auto jeoModel      = JeoModel{};
    jeoModel.colors    = fromJson(Type<std::vector<JeoColor>>{}, json["colors"]);
    jeoModel.tags      = json["tags"].as<std::vector<std::string>>();
    jeoModel.points    = fromJson(Type<std::vector<JeoPoint>>{}, json["points"]);
    jeoModel.lines     = fromJson(Type<std::vector<JeoLine>>{}, json["lines"]);
    jeoModel.arcs      = fromJson(Type<std::vector<JeoArc>>{}, json["arcs"]);
    jeoModel.polylines = fromJson(Type<std::vector<JeoPolyline>>{}, json["polylines"]);

    return jeoModel;
}