#include <algorithm>
#include <vector>
#include <iterator>
#include "gtest/gtest.h"
#include "ann/gauss_lsh.h"
#include "ann/linear_space.h"

typedef uint32_t ID;

template<typename _ForwardIterator>
void RandomFill(_ForwardIterator first, _ForwardIterator last) {
    boost::mt19937_64 prng_;
    boost::normal_distribution<float> gauss(0.0, 1.0);
    boost::variate_generator<boost::mt19937_64&,
        boost::normal_distribution<float>> rand_var(prng_, gauss);
    std::generate(first, last, rand_var);
}

unsigned int UpsertRandom(Space<ID>& indexer, ID id) {
    std::vector<float> vec(indexer.Dim());
    RandomFill(vec.begin(), vec.end());
    SpaceInput<ID> input;
    input.id = id;
    input.point = vec.data();
    return indexer.Upsert(input);
}

TEST(ann_test, lsh_upsert)
{
    LSHSpace<ID> indexer;
    indexer.Init(10);
    ASSERT_EQ(indexer.Size(), 0);
    ID id = 1;
    ASSERT_EQ(1, UpsertRandom(indexer, id));
    ASSERT_EQ(indexer.Size(), 1);
    std::vector<SpaceResult<ID>> results;
    indexer.GetNeighbors(id, 10, results);
    ASSERT_EQ(results.size(), 1);
}

TEST(ann_test, linear_upsert)
{
    LinearSpace<ID> indexer;
    indexer.Init(10);
    ASSERT_EQ(indexer.Size(), 0);
    ID id = 1;
    ASSERT_EQ(1, UpsertRandom(indexer, id));
    ASSERT_EQ(indexer.Size(), 1);
    std::vector<SpaceResult<ID>> results;
    indexer.GetNeighbors(id, 10, results);
    ASSERT_EQ(results.size(), 1);
}

TEST(ann_test, lsh_upsert_delete)
{
    LSHSpace<ID> indexer;
    indexer.Init(10);
    ASSERT_EQ(indexer.Size(), 0);
    ID id = 1;
    ASSERT_EQ(1, UpsertRandom(indexer, id));
    ASSERT_EQ(indexer.Size(), 1);
    indexer.Delete(id);
    ASSERT_EQ(indexer.Size(), 0);
    std::vector<SpaceResult<ID>> results;
    indexer.GetNeighbors(id, 10, results);
    ASSERT_EQ(results.size(), 0);
}
