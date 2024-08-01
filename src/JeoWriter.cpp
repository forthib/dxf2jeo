#include "JeoWriter.h"

#include "JeoModel.h"
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include <jsoncons/json.hpp>

namespace {
    auto toArray(const JeoColor& color) { return std::array{color.r, color.g, color.b}; }
    auto toArray(const JeoPoint& point) { return std::array{point.x, point.y, point.z}; }

    auto toArray(const JeoLine& line)
    {
        auto values = std::array<std::int64_t, 4>{};
        values[0]   = static_cast<std::int64_t>(line.firstPointIndex);
        values[1]   = static_cast<std::int64_t>(line.lastPointIndex);
        values[2]   = static_cast<std::int64_t>(line.colorIndex.value_or(-1));
        values[3]   = static_cast<std::int64_t>(line.tagIndex.value_or(-1));
        return values;
    }

    auto toArray(const JeoArc& arc)
    {
        auto values = std::array<std::int64_t, 6>{};
        values[0]   = static_cast<std::int64_t>(arc.centerIndex);
        values[1]   = static_cast<std::int64_t>(arc.firstPointIndex);
        values[2]   = static_cast<std::int64_t>(arc.lastPointIndex);
        values[3]   = static_cast<std::int64_t>(arc.direct ? 1 : 0);
        values[4]   = static_cast<std::int64_t>(arc.colorIndex.value_or(-1));
        values[5]   = static_cast<std::int64_t>(arc.tagIndex.value_or(-1));
        return values;
    }

    auto toVector(const JeoPolyline& polyline)
    {
        auto values = std::vector<std::int64_t>{};
        values.reserve(polyline.pointIndexes.size() + 2);
        for (const auto pointIndex : polyline.pointIndexes)
            values.push_back(static_cast<std::int64_t>(pointIndex));
        values.push_back(static_cast<std::int64_t>(polyline.colorIndex.value_or(-1)));
        values.push_back(static_cast<std::int64_t>(polyline.tagIndex.value_or(-1)));
        return values;
    }

    template<typename T> auto toArrays(const std::vector<T>& values)
    {
        using ValueArray = std::remove_cv_t<std::remove_reference_t<decltype(toArray(values.front()))>>;

        auto valueArrays = std::vector<ValueArray>(values.size());
        std::transform(values.begin(), values.end(), valueArrays.begin(), [](const auto& value) { return toArray(value); });
        return valueArrays;
    }

    template<typename T> auto toVectors(const std::vector<T>& values)
    {
        using ValueVector = std::remove_cv_t<std::remove_reference_t<decltype(toVector(values.front()))>>;

        auto valueVectors = std::vector<ValueVector>(values.size());
        std::transform(values.begin(), values.end(), valueVectors.begin(), [](const auto& value) { return toVector(value); });
        return valueVectors;
    }
}

void writeJeo(const JeoModel& model, const std::filesystem::path& filePath)
{
    auto out = std::ofstream{filePath};
    if (!out.is_open())
        throw std::runtime_error{fmt::format("unable to write file {}", filePath.string())};

    auto jsonVersion = jsoncons::ojson{};
    jsonVersion.insert_or_assign("major", 1);
    jsonVersion.insert_or_assign("minor", 0);

    auto json = jsoncons::ojson{};
    json.insert_or_assign("version", jsonVersion);
    json.insert_or_assign("colors", toArrays(model.colors));
    json.insert_or_assign("tags", model.tags);
    json.insert_or_assign("points", toArrays(model.points));
    json.insert_or_assign("lines", toArrays(model.lines));
    json.insert_or_assign("arcs", toArrays(model.arcs));
    json.insert_or_assign("polylines", toVectors(model.polylines));

    auto jsonOptions = jsoncons::json_options{};
    jsonOptions.precision(20);
    jsonOptions.array_array_line_splits(jsoncons::line_split_kind::same_line);
    json.dump(out, jsonOptions, jsoncons::indenting::indent);
}