#include <algorithm>
#include <vector>
#include <iterator>
#include "gtest/gtest.h"
#include "ann/gauss_lsh.h"


typedef uint32_t ID;

template<typename _ForwardIterator>
void RandomFill(_ForwardIterator first, _ForwardIterator last) {
    uint64_t seed = 0;
    boost::mt19937_64 prng_(seed);
    boost::normal_distribution<float> gauss(0.0, 1.0);
    boost::variate_generator<boost::mt19937_64&,
        boost::normal_distribution<float>> rand_var(prng_, gauss);
    std::generate(first, last, rand_var);
}


TEST(ann_test, mean_variance)
{
    LSHSpace<ID> indexer;
    indexer.Init(10);

    std::vector<float> vec;
    vec.reserve(10);
    RandomFill(vec.begin(), vec.end());
    SpaceInput<ID> input;
    input.id = 1;
    input.point = vec.data();
    indexer.Upsert(input);

    std::vector<SpaceResult<ID>> results;
    results.reserve(10);
    indexer.GetNeighbors(input.id, 10, results);

    ASSERT_EQ(results.size(), 1);
}
