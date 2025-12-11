#include "Dxf2Jeo.h"
#include "DxfModel.h"
#include "DxfReader.h"
#include "DxfWriter.h"
#include "Jeo2Dxf.h"
#include "JeoModel.h"
#include "JeoReader.h"
#include "JeoWriter.h"
#include <filesystem>
#include <gtest/gtest.h>

namespace {

    std::filesystem::path getAssetDir() { return TEST_ASSET_DIR; }

    TEST(dxf2jeotests, test1)
    {
        const auto inputPath = getAssetDir() / "test1.jeo";
        const auto jeoModel  = readJeo(inputPath);
        const auto dxfModel  = convertToDxf(jeoModel);

        ASSERT_EQ(dxfModel.arcs.size(), 1);
        ASSERT_NEAR(dxfModel.arcs[0].theta1, 3.1415911628956970, 1e-12);
        ASSERT_NEAR(dxfModel.arcs[0].theta2, 3.5779254259437274, 1e-12);
    }

    TEST(dxf2jeotests, test2)
    {
        const auto inputPath = getAssetDir() / "test2.jeo";
        const auto jeoModel  = readJeo(inputPath);
        const auto dxfModel  = convertToDxf(jeoModel);

        ASSERT_EQ(dxfModel.arcs.size(), 1);
        ASSERT_NEAR(dxfModel.arcs[0].theta1, 1.0808390005411683, 1e-12);
        ASSERT_NEAR(dxfModel.arcs[0].theta2, 2.0607536530486250, 1e-12);
    }

    TEST(dxf2jeotests, test3)
    {
        const auto inputPath = getAssetDir() / "test3.dxf";
        const auto dxfModel  = readDxf(inputPath);

        ASSERT_EQ(dxfModel.lines.size(), 1);
        ASSERT_TRUE(dxfModel.lines[0].peURL.has_value());
        ASSERT_EQ(dxfModel.lines[0].peURL.value(), "hello");
    }

    TEST(dxf2jeotests, test4)
    {
        const auto inputPath = getAssetDir() / "test4.dxf";
        const auto dxfModel  = readDxf(inputPath);
        const auto jeoModel  = convertToJeo(dxfModel);

        ASSERT_EQ(jeoModel.polylines.size(), 1);
        ASSERT_TRUE(jeoModel.polylines[0].bulges.has_value());
        ASSERT_EQ(jeoModel.polylines[0].bulges->size(), 4);
        ASSERT_NEAR(jeoModel.polylines[0].bulges->at(2), 1., 1e-15);
    }
}