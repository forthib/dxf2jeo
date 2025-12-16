#include "DxfReader.h"

#include "ArcUtils.h"
#include "DxfModel.h"
#include <algorithm>
#include <cmath>
#include <fmt/format.h>
#include <libdxfrw/libdxfrw.h>

namespace {

    DxfLayer convertLayer(const DRW_Layer& data)
    {
        auto layer  = DxfLayer{};
        layer.name  = data.name;
        layer.color = data.color;
        return layer;
    }

    std::optional<std::string> findPEURL(const DRW_Entity& data)
    {
        bool active = false;

        for (const auto& varData : data.extData) {
            if (varData->code() == 1001 && varData->type() == DRW_Variant::STRING)
                active = *varData->content.s == "PE_URL";
            else if (varData->code() == 1000 && varData->type() == DRW_Variant::STRING && active)
                return *varData->content.s;
        }
        return std::nullopt;
    }

    DxfEntity convertEntity(const DRW_Entity& data)
    {
        auto entity  = DxfEntity{};
        entity.layer = data.layer;
        if (data.color != DRW::ColorByLayer)
            entity.color = data.color;
        entity.peURL = findPEURL(data);
        return entity;
    }

    DxfCoord convertCoord(const DRW_Coord& data)
    {
        auto coord = DxfCoord{};
        coord.x    = data.x;
        coord.y    = data.y;
        coord.z    = data.z;
        return coord;
    }

    DxfLine convertLine(const DRW_Line& data)
    {
        auto line = DxfLine{convertEntity(data)};
        line.p1   = convertCoord(data.basePoint);
        line.p2   = convertCoord(data.secPoint);
        return line;
    }

    DxfArc convertArc(const DRW_Arc& data)
    {
        auto arc   = DxfArc{convertEntity(data)};
        arc.center = convertCoord(data.basePoint);
        arc.radius = data.radious;
        arc.theta1 = data.staangle;
        arc.theta2 = data.endangle;

        const auto direct = data.extPoint.z > 0.;
        normalize(arc.theta1, arc.theta2, direct);
        return arc;
    }

    DxfPolyline convertPolyline(const DRW_LWPolyline& data)
    {
        auto polyline = DxfPolyline{convertEntity(data)};
        auto bulges   = std::vector<double>{};
        for (auto i = 0; i < data.vertexnum; ++i) {
            auto coord = DxfCoord{};
            coord.x    = data.vertlist[i]->x;
            coord.y    = data.vertlist[i]->y;
            coord.z    = data.elevation;
            polyline.coords.push_back(coord);
            bulges.push_back(data.vertlist[i]->bulge);
        }
        if (std::any_of(bulges.begin(), bulges.end(), [](double bulge) { return std::fabs(bulge) > std::numeric_limits<double>::epsilon(); }))
            polyline.bulges = std::move(bulges);
        polyline.closed = data.flags & 1;

        return polyline;
    }

    std::unordered_map<std::string, std::int64_t> makeLayerNameToColorMap(const DxfModel& model)
    {
        auto map = std::unordered_map<std::string, std::int64_t>{};
        for (const auto& [layerName, color] : model.layers)
            map[layerName] = color;
        return map;
    }

    std::optional<std::int64_t> getColor(const DxfEntity& entity, const std::unordered_map<std::string, std::int64_t>& layerNameToColor)
    {
        if (entity.color)
            return entity.color;
        else if (const auto it = layerNameToColor.find(entity.layer); it != layerNameToColor.end())
            return it->second;
        else
            return std::nullopt;
    }

    DxfModel checkModel(DxfModel model)
    {
        const auto layerNameToColor = makeLayerNameToColorMap(model);
        for (auto& line : model.lines)
            line.color = getColor(line, layerNameToColor);
        for (auto& arc : model.arcs)
            arc.color = getColor(arc, layerNameToColor);
        for (auto& polyline : model.polylines)
            polyline.color = getColor(polyline, layerNameToColor);
        return model;
    }

    class DxfReaderInterface : public DRW_Interface
    {
      public:
        void addHeader(const DRW_Header*) override {}
        void addLType(const DRW_LType&) override {}
        void addLayer(const DRW_Layer& data) override { model_.layers.push_back(convertLayer(data)); }
        void addDimStyle(const DRW_Dimstyle&) override {}
        void addVport(const DRW_Vport&) override {}
        void addTextStyle(const DRW_Textstyle&) override {}
        void addAppId(const DRW_AppId&) override {}
        void addBlock(const DRW_Block&) override {}
        void setBlock(const int) override {}
        void endBlock() override {}
        void addPoint(const DRW_Point&) override {}
        void addLine(const DRW_Line& data) override { model_.lines.push_back(convertLine(data)); }
        void addRay(const DRW_Ray&) override {}
        void addXline(const DRW_Xline&) override {}
        void addArc(const DRW_Arc& data) override { model_.arcs.push_back(convertArc(data)); }
        void addCircle(const DRW_Circle&) override {}
        void addEllipse(const DRW_Ellipse&) override {}
        void addLWPolyline(const DRW_LWPolyline& data) override { model_.polylines.push_back(convertPolyline(data)); }
        void addPolyline(const DRW_Polyline&) override {}
        void addSpline(const DRW_Spline*) override {}
        void addKnot(const DRW_Entity&) override {}
        void addInsert(const DRW_Insert&) override {}
        void addTrace(const DRW_Trace&) override {}
        void add3dFace(const DRW_3Dface&) override {}
        void addSolid(const DRW_Solid&) override {}
        void addMText(const DRW_MText&) override {}
        void addText(const DRW_Text&) override {}
        void addDimAlign(const DRW_DimAligned*) override {}
        void addDimLinear(const DRW_DimLinear*) override {}
        void addDimRadial(const DRW_DimRadial*) override {}
        void addDimDiametric(const DRW_DimDiametric*) override {}
        void addDimAngular(const DRW_DimAngular*) override {}
        void addDimAngular3P(const DRW_DimAngular3p*) override {}
        void addDimOrdinate(const DRW_DimOrdinate*) override {}
        void addLeader(const DRW_Leader*) override {}
        void addHatch(const DRW_Hatch*) override {}
        void addViewport(const DRW_Viewport&) override {}
        void addImage(const DRW_Image*) override {}
        void linkImage(const DRW_ImageDef*) override {}
        void addComment(const char*) override {}
        void addPlotSettings(const DRW_PlotSettings*) override {}

        void writeHeader(DRW_Header& data) override {}
        void writeBlocks() override {}
        void writeBlockRecords() override {}
        void writeEntities() override {}
        void writeLTypes() override {}
        void writeLayers() override {}
        void writeTextstyles() override {}
        void writeVports() override {}
        void writeDimstyles() override {}
        void writeObjects() override {}
        void writeAppId() override {}

        DxfModel model() const { return checkModel(model_); }

      private:
        DxfModel model_;
    };
}

DxfModel readDxf(const std::filesystem::path& filePath)
{
    const auto filePathStr = filePath.string();

    auto dxfInterf = DxfReaderInterface{};
    auto dxfrw     = dxfRW(filePathStr.c_str());
    if (!dxfrw.read(&dxfInterf, true))
        throw std::runtime_error{fmt::format("unable to read file {}", filePath.string())};
    return dxfInterf.model();
}