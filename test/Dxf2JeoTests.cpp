#include "Dxf2Jeo.h"
#include "DxfModel.h"
#include "DxfReader.h"
#include "DxfWriter.h"
#include "Jeo2Dxf.h"
#include "JeoModel.h"
#include "JeoReader.h"
#include <filesystem>
#include <gtest/gtest.h>

namespace {

    std::filesystem::path getAssetDir() { return TEST_ASSET_DIR; }

    TEST(dex2jeotests, test1)
    {
        const auto inputPath = getAssetDir() / "test1.jeo";
        const auto jeoModel = readJeo(inputPath);
        const auto dxfModel = convertToDxf(jeoModel);

        ASSERT_EQ(dxfModel.arcs.size(), 1);
        ASSERT_NEAR(dxfModel.arcs[0].theta1, 3.1415911628956970, 1e-12);
        ASSERT_NEAR(dxfModel.arcs[0].theta2, 3.5779254259437274, 1e-12);
    }
}