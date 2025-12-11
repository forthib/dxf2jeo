#include "JeoWriter.h"

#include "JeoModel.h"
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <jsoncons/json.hpp>

namespace {
    auto toJson(const JeoColor& color) { return std::array{color.r, color.g, color.b}; }
    auto toJson(const JeoPoint& point) { return std::array{point.x, point.y, point.z}; }

    auto toJson(const JeoEntity& entity)
    {
        auto json = jsoncons::ojson{};
        if (entity.colorIndex)
            json.insert_or_assign("color", entity.colorIndex.value());
        if (entity.tagIndex)
            json.insert_or_assign("tag", entity.tagIndex.value());
        return json;
    }

    auto toJson(const JeoLine& line)
    {
        auto json = toJson(static_cast<const JeoEntity&>(line));
        json.insert_or_assign("points", std::array{line.firstPointIndex, line.lastPointIndex});
        return json;
    }

    auto toJson(const JeoArc& arc)
    {
        auto json = toJson(static_cast<const JeoEntity&>(arc));
        json.insert_or_assign("points", std::array{arc.centerIndex, arc.firstPointIndex, arc.lastPointIndex});
        json.insert_or_assign("direct", arc.direct);
        return json;
    }

    auto toJson(const JeoPolyline& polyline)
    {
        auto json = toJson(static_cast<const JeoEntity&>(polyline));
        json.insert_or_assign("points", polyline.pointIndexes);
        if (polyline.bulges)
            json.insert_or_assign("bulges", polyline.bulges.value());
        json.insert_or_assign("closed", polyline.closed);
        return json;
    }

    template<typename T> auto toJson(const std::vector<T>& elements)
    {
        auto json = jsoncons::ojson::make_array();
        json.reserve(elements.size());
        for (const auto& element : elements)
            json.push_back(toJson(element));
        return json;
    }
}

void writeJeo(const JeoModel& model, const std::filesystem::path& filePath)
{
    auto out = std::ofstream{filePath};
    if (!out.is_open())
        throw std::runtime_error{fmt::format("unable to write file {}", filePath.string())};

    auto jsonVersion = jsoncons::ojson{};
    jsonVersion.insert_or_assign("major", 2);
    jsonVersion.insert_or_assign("minor", 0);

    auto json = jsoncons::ojson{};
    json.insert_or_assign("version", jsonVersion);
    json.insert_or_assign("colors", toJson(model.colors));
    json.insert_or_assign("tags", model.tags);
    json.insert_or_assign("points", toJson(model.points));
    json.insert_or_assign("lines", toJson(model.lines));
    json.insert_or_assign("arcs", toJson(model.arcs));
    json.insert_or_assign("polylines", toJson(model.polylines));

    auto jsonOptions = jsoncons::json_options{};
    jsonOptions.precision(20);
    jsonOptions.array_array_line_splits(jsoncons::line_split_kind::same_line);
    json.dump(out, jsonOptions, jsoncons::indenting::indent);
}