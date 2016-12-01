#pragma once

#include <cmath>
#include <iostream>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include <boost/bind.hpp>
#include <boost/nondet_random.hpp>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

#include <Eigen/Dense>

#include "common/ann_util.h"
#include "common/base64.h"
#include "ann/space.h"


using std::multimap;
using std::unordered_map;
using std::unordered_set;
using std::vector;
using ann::util::ProgressBar;

namespace /**** begin namespace ****/
{

template<typename A, typename B>
std::pair<B,A> flip_pair(const std::pair<A,B> &p)
{
    return std::pair<B,A>(p.second, p.first);
}

template<typename A, typename B, template<class,class,class...> class M, class... Args>
std::multimap<B,A> flip_map(const M<A,B,Args...> &src)
{
    std::multimap<B,A> dst;
    std::transform(src.begin(), src.end(),
                   std::inserter(dst, dst.begin()),
                   flip_pair<A,B>);
    return dst;
}

template<typename ID>
using Bucket = unordered_map<std::string, std::unordered_set<ID>>;

} /**** end namespace ****/

template <typename ID>
class LSHSpace : public Space<ID> {
    public:
        LSHSpace(uint64_t seed=0)
            : prng_(seed)
        {};
        ~LSHSpace();

        void Init(size_t nb_dims) override;

        void Config(size_t nb_dims, size_t L=15, size_t k=32, float w=0.5, size_t search_k=0);

        void Clear() override;

        unsigned int Delete(const ID& id) override;

        unsigned int Upsert(const SpaceInput<ID>& input) override;

        void GetNeighbors(const float* point, size_t nb_results,
                vector<SpaceResult<ID>>& results) const override;

        void GetNeighbors(const ID& id, size_t nb_results,
                vector<SpaceResult<ID>>& results) const override;

        void GraphToStream(std::ostream& out, size_t nb_results) const override;

        // Get the number of elements stored.
        size_t Size() const override { return ids_.size(); }

        // Get Dimensionality
        size_t Dim() const override { return ndim_; }

    private:

        size_t ndim_;
        vector<ID> ids_;

        // Methods
        void _InitBuckets();
        void _InitTables();
        void _InitOffsets();
        void _IterBuckets(const Eigen::VectorXf &vec,
                std::function<void(size_t, const std::string&)> func) const;

        void GetNeighbors(const Eigen::VectorXf &vec, size_t nb_results,
            vector<SpaceResult<ID>>& results) const;

        // dynamically allocated members
        // actual data stored
        unordered_map<ID, size_t> id2index_;
        vector<Eigen::VectorXf> points_;

        // dynamically allocated members (2)
        //
        // not using aligned allocator provided by Eigen
        // because it is for fixed-size types only.
        vector<Eigen::MatrixXf> tables_;
        vector<Eigen::VectorXf> offsets_;
        vector<Bucket<ID>> buckets_;

        // constructor params
        size_t L_;
        size_t k_;
        float w_;
        size_t search_k_;

        // members initialized in the initialization list
        boost::mt19937_64 prng_;

};

template <typename ID>
LSHSpace<ID>::~LSHSpace() {}

template <typename ID>
void LSHSpace<ID>::Config(size_t nb_dims, size_t L, size_t k, float w, size_t search_k)
{
    // create placeholders for random vectors
    ndim_ = nb_dims;
    L_ = L;
    k_ = k;
    w_ = w;
    search_k_ = search_k;

    ndim_ = nb_dims;
    _InitTables();
    _InitOffsets();
    _InitBuckets();
}

template <typename ID>
void LSHSpace<ID>::Init(size_t nb_dims) {
    Config(nb_dims);
}

template <typename ID>
void LSHSpace<ID>::Clear() {
    id2index_.clear();
    ids_.clear();
    points_.clear();
    tables_.clear();
    offsets_.clear();

    std::vector<Eigen::VectorXf> points_;
}

template <typename ID>
unsigned int LSHSpace<ID>::Delete(const ID& id) {
    if (ids_.empty()) {
        return 0;
    }

    // Look up the ID.
    auto it = id2index_.find(id);
    if (it == id2index_.end()) {
        return 0;
    }
    size_t curr_idx = it->second;
    size_t last_idx = ids_.size() - 1;

    // Swap with the end and resize by one.
    ids_[curr_idx] = ids_[last_idx];
    ids_.resize(last_idx);

    points_[curr_idx] = points_[last_idx];
    points_.resize(last_idx);

    id2index_[ids_[last_idx]] = curr_idx;
    id2index_.erase(it);

    // TODO: add code to delete LSH indices
    return 1;
}

template <typename ID>
void LSHSpace<ID>::_IterBuckets(
        const Eigen::VectorXf &vec,
        std::function<void(size_t, const std::string&)> func) const
{
    for (size_t bucket_idx=0; bucket_idx < L_; ++bucket_idx) {
        auto& table = tables_[bucket_idx];
        auto& offset = offsets_[bucket_idx];

        auto proj_whole = ((table * vec) + offset) / w_;
        auto proj_floor = proj_whole.unaryExpr(std::ptr_fun(::floor));
        Eigen::VectorXi quantized = proj_floor.template cast<int>();

        //std::cout << quantized.transpose() << std::endl;

        // TODO: use byte array instead of string
        size_t key_size = quantized.size() * sizeof(int);
        const std::string key(reinterpret_cast<const char*>(quantized.data()), key_size);

        func(bucket_idx, key);
    }
}

float sumVec(const float* vec, size_t n) {
    float res = 0.0;
    const float *end = vec + n;
    for (; vec < end; ++vec) {
        res += *vec;
    }
    return res;
}

template <typename ID>
unsigned int LSHSpace<ID>::Upsert(const SpaceInput<ID>& input) {

    // Reject NaN entries.
    if (!isfinite_xf(input.point, ndim_)) {
        return 0;
    }

    std::vector<float> vec;
    vec.reserve(ndim_);
    for (size_t i = 0; i < ndim_; ++i) {
        vec.emplace_back(input.point[i]);
    }
    auto evec = Eigen::Map<Eigen::VectorXf>(vec.data(), ndim_);

    // Reject inputs whose norm equals zero
    if (evec.norm() == 0.0) {
        return 0;
    }

    const ID& id = input.id;
    Delete(id);

    size_t idx = ids_.size();
    ids_.emplace_back(id);
    id2index_[id] = idx;

    evec.normalize();
    points_.emplace_back(evec);

    // fill buckets
    _IterBuckets(evec, [this, &id] (size_t bucket_idx, const std::string &key) {
        auto& bucket = this->buckets_[bucket_idx];
        auto it = bucket.find(key);
        if (it != bucket.end()) {
            it->second.insert(id);
        }
        else {
            unordered_set<ID> value;
            value.insert(id);
            bucket.insert(std::make_pair(key, value));
        }
    });
    return 1;
}

template <typename ID>
void LSHSpace<ID>::GetNeighbors(const Eigen::VectorXf &evec, size_t nb_results,
        vector<SpaceResult<ID>>& results) const
{
    std::unordered_map<ID, uint32_t> counter;

    _IterBuckets(evec, [this, &counter] (size_t bucket_idx, const std::string &key) {
        auto& bucket = this->buckets_[bucket_idx];
        auto it = bucket.find(key);
        if (it != bucket.end()) {
            auto& ids = it->second;
            for (auto& id : ids) {
                counter[id]++;
            }
        }
    });

    // first sort by the number of matching buckets
    // (this is done using flip_map)
    std::multimap<uint32_t, ID> dst = flip_map(counter);
    size_t i;
    size_t search_k = (search_k_ == 0) ? L_ * nb_results : search_k_;
    size_t num_candidates = std::min(search_k, dst.size());
    auto cit = dst.rbegin();
    std::vector<SpaceResult<ID>> candidates;
    candidates.reserve(num_candidates);
    for (i = 0; i < num_candidates; ++i) {
        ID& id = cit->second;
        auto it = id2index_.find(id);
        if (it != id2index_.end()) {
            auto idx = it->second;
            SpaceResult<ID> slot;
            slot.id = id;
            slot.dist = 1.0 - evec.dot(points_[idx]);  // calculate distance
            candidates.emplace_back(slot);
        }
        ++cit;
    }
    // next sort by distances in ascending order
    std::sort(candidates.begin(), candidates.end());
    results.reserve(nb_results);
    auto limit = std::min(candidates.size(), results.capacity());
    auto it = candidates.begin();
    for (i = 0; i < limit; ++i) {
        results.emplace_back(*it);
        ++it;
    }
}

template <typename ID>
void LSHSpace<ID>::GetNeighbors(const float* point, size_t nb_results,
        vector<SpaceResult<ID>>& results) const
{
    std::vector<float> vec;
    vec.reserve(ndim_);
    for (size_t i = 0; i < ndim_; ++i) {
        vec.emplace_back(point[i]);
    }

    auto evec = Eigen::Map<Eigen::VectorXf>(vec.data(), ndim_);
    evec.normalize();

    GetNeighbors(evec, nb_results, results);
}

template <typename ID>
void LSHSpace<ID>::GetNeighbors(const ID& id, size_t nb_results,
        vector<SpaceResult<ID>>& results) const
{
    auto it = id2index_.find(id);
    if (it != id2index_.end()) {
        auto index = it->second;
        auto& vec = points_[index];
        GetNeighbors(vec, nb_results, results);
    }
}

template <typename ID>
void LSHSpace<ID>::_InitTables() {
    boost::normal_distribution<float> gauss(0.0, 1.0);
    boost::variate_generator<boost::mt19937_64&,
        boost::normal_distribution<float>> rand_var(prng_, gauss);

    tables_.clear();
    tables_.reserve(L_);
    for (size_t i=0; i < L_; ++i) {
        std::vector<float> data(k_ * ndim_);
        std::generate(data.begin(), data.end(), rand_var);
        auto mat = Eigen::Map<Eigen::MatrixXf>(data.data(), k_, ndim_);
        for (size_t j=0; j < k_; ++j) {
            auto vec = mat.row(j);
            vec.normalize();
        }
        tables_.emplace_back(mat);
    }
}

template <typename ID>
void LSHSpace<ID>::_InitBuckets() {
    buckets_.clear();
    buckets_.reserve(L_);
    for (size_t i = 0; i < L_; ++i) {
        Bucket<ID> bucket;
        buckets_.emplace_back(bucket);
    }
}

template <typename ID>
void LSHSpace<ID>::_InitOffsets() {
    boost::uniform_real<float> uniform(0.0, w_);
    boost::variate_generator<boost::mt19937_64&,
        boost::uniform_real<float>> rand_var(prng_, uniform);

    offsets_.clear();
    offsets_.reserve(L_);
    for (size_t i=0; i < L_; ++i) {
        std::vector<float> data(k_* ndim_);
        std::generate(data.begin(), data.end(), rand_var);
        auto offset = Eigen::Map<Eigen::VectorXf>(data.data(), k_);
        offsets_.emplace_back(offset);
    }
}

template <typename ID>
void LSHSpace<ID>::GraphToStream(std::ostream& out, size_t nb_results) const {
    // Iterate over all ids stored
    size_t total = ids_.size();
    auto progBar = ProgressBar(total);
    #pragma omp parallel for shared(progBar)
    for (size_t i = 0; i < total; ++i) {
        const ID& id = ids_[i];
        vector<SpaceResult<ID>> results;
        GetNeighbors(id, nb_results, results);
        #pragma omp critical
        {
        progBar.update();
        WriteResults(out, id, results);
        }
    }
}
