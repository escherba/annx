#include <cassert>

// unix headers
#include <fstream>
#include <string>

#include <boost/filesystem/fstream.hpp>

#include "ann/space.h"
#include "ann/spark_rdd.h"
#include "ann/linear_space.h"


namespace spark {

    namespace fs = boost::filesystem;

    bool parse_SparkLine(const std::string& line, unsigned int& rid, std::vector<float>& v)
    {
        v.clear();
        if (parse_List(line.begin(), line.end(), rid, v)) {
            return true;
        }
        v.clear();
        if (parse_WrappedArray(line.begin(), line.end(), rid, v)) {
            return true;
        }
        return false;
    }

    void LoadFile(std::istream& infile, Space<AnyID>* space)
    {
        std::string line;
        size_t num_parsed = 0;
        size_t num_loaded = 0;
        for(size_t line_id = 0; std::getline(infile, line); line_id++) {
            unsigned int rid;
            std::vector<float> v;
            bool ok = parse_SparkLine(line, rid, v);
            if (ok) {
                // we have item id and item embedding at this point
                // 1) create a SpaceInput struct
                //
                SpaceInput<unsigned int> si = {rid, v.data()};
                num_loaded += space->Upsert(si);
                num_parsed++;
            } else {
                // report that the match didn't succeed
                std::cerr << "Failed at line " << line_id << ": " << line << std::endl;
                break;
            }
        }
        std::cerr << "(" << num_parsed << " lines parsed, "
            << num_parsed - num_loaded << " skipped)" << std::endl;
    }

    void LoadFiles(const std::string& path, Space<AnyID>* space)
    {
        if (path == "-") {
            std::cerr << "Reading from stdin..." << std::endl;
            spark::LoadFile(std::cin, space);
        } else if (fs::is_directory(path)) {
            for (fs::directory_iterator itr(path); itr != fs::directory_iterator(); ++itr) {
                const fs::path filepath = itr->path();
                if (fs::is_regular_file(filepath)) {
                    std::cerr << "Processing " << filepath.filename() << "... ";
                    std::ifstream infile(filepath.c_str());
                    spark::LoadFile(infile, space);
                }
            }
        } else if (fs::is_regular_file(path)) {
            const fs::path filepath(path);
            std::ifstream infile(filepath.c_str());
            spark::LoadFile(infile, space);
        } else {
            std::cerr << path << ": No such file or directory" << std::endl;
        }
    }
}
