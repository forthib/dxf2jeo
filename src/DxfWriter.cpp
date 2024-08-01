#include "DxfWriter.h"

#include "DxfModel.h"
#include <fmt/format.h>
#include <libdxfrw/libdxfrw.h>

namespace {

    template<typename DrwEntity> DrwEntity convertEntity(const DxfEntity& entity)
    {
        auto data  = DrwEntity{};
        data.layer = entity.layer;
        data.color = static_cast<int>(entity.color.value_or(DRW::ColorByLayer));
        if (entity.peURL) {
            data.extData.resize(2);
            data.extData[0] = std::make_shared<DRW_Variant>(1001, "PE_URL");
            data.extData[1] = std::make_shared<DRW_Variant>(1000, entity.peURL->c_str());
        }
        return data;
    }

    DRW_Coord convertCoord(const DxfCoord& coord)
    {
        auto data = DRW_Coord{};
        data.x    = coord.x;
        data.y    = coord.y;
        data.z    = coord.z;
        return data;
    }

    DRW_Line convertLine(const DxfLine& line)
    {
        auto data      = convertEntity<DRW_Line>(line);
        data.basePoint = convertCoord(line.p1);
        data.secPoint  = convertCoord(line.p2);
        return data;
    }

    DRW_Arc convertArc(const DxfArc& arc)
    {
        auto data      = convertEntity<DRW_Arc>(arc);
        data.basePoint = convertCoord(arc.center);
        data.radious   = arc.radius;
        data.staangle  = arc.theta1;
        data.endangle  = arc.theta2;
        return data;
    }

    DRW_LWPolyline convertPolyline(const DxfPolyline& polyline)
    {
        auto data = convertEntity<DRW_LWPolyline>(polyline);
        data.vertlist.reserve(polyline.coords.size());
        for (const auto& coord : polyline.coords) {
            data.vertlist.push_back(std::make_shared<DRW_Vertex2D>(coord.x, coord.y, 0));
            data.elevation = coord.z;
        }
        if (polyline.closed)
            data.flags |= 1;
        return data;
    }

    DRW_Layer convertLayer(const DxfLayer& layer)
    {
        auto data  = DRW_Layer{};
        data.name  = layer.name;
        data.color = static_cast<int>(layer.color);
        return data;
    }

    class DxfWriterInterface : public DRW_Interface
    {
      public:
        DxfWriterInterface(const DxfModel& model, dxfRW& dxfrw) : model_{&model}, dxfrw_{&dxfrw} {}

        void addHeader(const DRW_Header*) override {}
        void addLType(const DRW_LType&) override {}
        void addLayer(const DRW_Layer& data) override {}
        void addDimStyle(const DRW_Dimstyle&) override {}
        void addVport(const DRW_Vport&) override {}
        void addTextStyle(const DRW_Textstyle&) override {}
        void addAppId(const DRW_AppId&) override {}
        void addBlock(const DRW_Block&) override {}
        void setBlock(const int) override {}
        void endBlock() override {}
        void addPoint(const DRW_Point&) override {}
        void addLine(const DRW_Line& data) override {}
        void addRay(const DRW_Ray&) override {}
        void addXline(const DRW_Xline&) override {}
        void addArc(const DRW_Arc& data) override {}
        void addCircle(const DRW_Circle&) override {}
        void addEllipse(const DRW_Ellipse&) override {}
        void addLWPolyline(const DRW_LWPolyline& data) override {}
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

        void writeEntities() override
        {
            for (const auto& line : model_->lines) {
                auto data = convertLine(line);
                dxfrw_->writeLine(&data);
            }
            for (const auto& arc : model_->arcs) {
                auto data = convertArc(arc);
                dxfrw_->writeArc(&data);
            }
            for (const auto& polyline : model_->polylines) {
                auto data = convertPolyline(polyline);
                dxfrw_->writeLWPolyline(&data);
            }
        }

        void writeLTypes() override {}

        void writeLayers() override
        {
            for (const auto& layer : model_->layers) {
                auto data = convertLayer(layer);
                dxfrw_->writeLayer(&data);
            }
        }

        void writeTextstyles() override {}
        void writeVports() override {}
        void writeDimstyles() override {}
        void writeObjects() override {}
        void writeAppId() override {}

      private:
        const DxfModel* model_;
        dxfRW*          dxfrw_;
    };
}

void writeDxf(const DxfModel& model, const std::filesystem::path& filePath)
{
    const auto filePathStr = filePath.string();

    auto dxfrw     = dxfRW(filePathStr.c_str());
    auto dxfInterf = DxfWriterInterface{model, dxfrw};
    if (!dxfrw.write(&dxfInterf, DRW::AC1027, false))
        throw std::runtime_error{fmt::format("unable to write file {}", filePath.string())};
}