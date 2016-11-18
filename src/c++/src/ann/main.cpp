#include <iostream>
#include <gflags/gflags.h>

#include "ann/gauss_lsh.h"
#include "ann/spark_rdd.h"
#include "ann/space.h"


DEFINE_bool(verbose, false, "Display program name before message");
DEFINE_string(algo, "linear", "Which algo to use");
DEFINE_string(input, "-", "input file or directory");
DEFINE_uint64(rank, 128, "Embedding rank");
DEFINE_uint64(seed, 0, "Random seed");
DEFINE_double(w, 0.5, "w-value");
DEFINE_uint64(L, 15, "L-value");
DEFINE_uint64(k, 32, "k-value");
DEFINE_uint64(n_neighbors, 10, "number of neighbors to return");
DEFINE_uint64(search_k, 0, "k-value (if 0, then equal to L * n_neighbors");


using spark::LoadFiles;


int main(int argc, char *argv[])
{
    gflags::SetUsageMessage("some usage message");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    if (FLAGS_verbose) std::cout << gflags::ProgramInvocationShortName() << ": ";
    gflags::ShutDownCommandLineFlags();

    // // Test LSH
    // auto hasher = LSH<float>(15, 32, 128);
    // hasher.SetSeed(FLAGS_seed);
    // hasher.Init(0.5);

    //Space<AnyID>* embedding_space_{nullptr};
    LSHSpace<uint32_t> embedding_space_;
    embedding_space_.Init(128);

    spark::LoadFiles(FLAGS_input.c_str(), &embedding_space_);
    embedding_space_.MakeGraph(FLAGS_n_neighbors);

    return 0;
}
