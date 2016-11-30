#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>

#include <gflags/gflags.h>
#include <Eigen/Core>

#include "ann/linear_space.h"
#include "ann/gauss_lsh.h"
#include "ann/spark_rdd.h"
#include "ann/space.h"


DEFINE_bool(verbose, false, "Display program name before message");
DEFINE_string(algo, "lsh", "Which algo to use (choices: lsh, linear)");
DEFINE_string(input, "-", "input file or directory");
DEFINE_string(output, "-", "output file");
DEFINE_uint64(rank, 128, "Embedding rank");
DEFINE_uint64(seed, 0, "Random seed");
DEFINE_double(w, 0.5, "w-value");
DEFINE_uint64(L, 15, "L-value");
DEFINE_uint64(k, 32, "k-value");
DEFINE_uint64(n_neighbors, 10, "number of neighbors to return");
DEFINE_uint64(search_k, 0, "k-value (if 0, then equal to L * n_neighbors");


using spark::LoadFiles;

void do_lsh() {
    LSHSpace<uint32_t> space_;
    space_.Init(128);

    auto t0 = std::clock();
    spark::LoadFiles(FLAGS_input, &space_);
    auto t1 = std::clock();
    double d1 = (t1 - t0) / (double) CLOCKS_PER_SEC;
    std::cerr << "time to load files: " << d1 << " sec" << std::endl;

    space_.GraphToPath(FLAGS_output, FLAGS_n_neighbors);
    auto t2 = std::clock();
    double d2 = (t2 - t1) / (double) CLOCKS_PER_SEC;
    std::cerr << "time to create graph: " << d2 << " sec" << std::endl;
}

void do_linear() {
    LinearSpace<uint32_t> space_;
    space_.Init(128);

    auto t0 = std::clock();
    spark::LoadFiles(FLAGS_input, &space_);
    auto t1 = std::clock();
    double d1 = (t1 - t0) / (double) CLOCKS_PER_SEC;
    std::cerr << "time to load files: " << d1 << " sec" << std::endl;

    space_.GraphToPath(FLAGS_output, FLAGS_n_neighbors);
    auto t2 = std::clock();
    double d2 = (t2 - t1) / (double) CLOCKS_PER_SEC;
    std::cerr << "time to create graph: " << d2 << " sec" << std::endl;
}


int main(int argc, char *argv[])
{
    std::stringstream usage;
    usage << argv[0] << " -input <path> -output <path>";
    gflags::SetUsageMessage(usage.str());
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    if (FLAGS_verbose) std::cout << gflags::ProgramInvocationShortName() << ": ";
    gflags::ShutDownCommandLineFlags();

    Eigen::initParallel();

    if (FLAGS_algo == "linear") {
        do_linear();
    }
    else if (FLAGS_algo == "lsh") {
        do_lsh();
    }
    else {
        std::cerr << "Unknown algo parameter: " << FLAGS_algo << std::endl;
        return 1;
    }

    return 0;
}
