#include "Dxf2JeoVersion.h"
#include "DxfModel.h"
#include "DxfWriter.h"
#include "Jeo2Dxf.h"
#include "JeoModel.h"
#include "JeoReader.h"
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>

namespace {
    auto getCLOptions()
    {
        auto options = cxxopts::Options{"jeo2dxf", "Converts a 2D .dxf file into Geometric Json .jeo file"};
        options.add_options()                                                   //
            ("i,input", "Input JEO file path", cxxopts::value<std::string>())   //
            ("o,output", "Output DXF file path", cxxopts::value<std::string>()) //
            ("v,version", "Display jeo2dxf version")                            //
            ("h,help", "Display this help");
        return options;
    }

    int help()
    {
        fmt::print(std::cout, "{}\n", getCLOptions().help());
        return 0;
    }

    int version()
    {
        fmt::print(std::cout, "jeo2dxf version {}.{}.{}\n", DXF2JEO_VERSION_MAJOR, DXF2JEO_VERSION_MINOR, DXF2JEO_VERSION_PATCH);
        return 0;
    }

    template<typename... Args> int error(std::string_view fmt, Args&&... args)
    {
        fmt::print(std::cerr, fmt::runtime(fmt::format("Error: {}\n", fmt)), std::forward<Args>(args)...);
        fmt::print("---------------------------------------\n");
        fmt::print(std::cerr, "{}\n", getCLOptions().help());
        return -1;
    }

    int run(int argc, char** argv)
    {
        try {
            const auto result = getCLOptions().parse(argc, argv);

            if (result.count("help"))
                return help();

            if (result.count("version"))
                return version();

            if (result.count("input") == 0)
                return error("input file path must be provided");

            if (result.count("output") == 0)
                return error("output file path must be provided");

            const auto inputPath = std::filesystem::path{result["input"].as<std::string>()};
            if (!std::filesystem::is_regular_file(inputPath))
                return error("output file is not a regular file: {}", inputPath.string());

            const auto outputPath = std::filesystem::path{result["output"].as<std::string>()};
            create_directories(outputPath.parent_path());

            const auto jeoModel = readJeo(inputPath);
            const auto dxfModel = convertToDxf(jeoModel);
            writeDxf(dxfModel, outputPath);

            return 0;
        }
        catch (const std::exception& e) {
            return error(e.what());
        }
        catch (...) {
            return error("unknown exception");
        }
    }
}

int main(int argc, char** argv)
{
    try {
        run(argc, argv);
        return 0;
    }
    catch (...) {
        return -1;
    }
}