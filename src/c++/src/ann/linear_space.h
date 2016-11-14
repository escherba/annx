#pragma once

#include <algorithm>
#include <cmath>
#include <pthread.h>
#include <set>
#include <thread>
#include <string>
#include <unordered_map>
#include <vector>

#include "ann/space.h"

using std::unordered_map;
using std::vector;

using std::isnan;
using std::set;


template <typename ID>
class LinearSpace : public Space<ID> {
  public:
    LinearSpace();
    ~LinearSpace();

    void Init(size_t nb_dims);

    void Clear();

    unsigned int Delete(const ID& id);

    unsigned int Upsert(const SpaceInput<ID>& input);

    void GetNeighbors(const float* point, size_t nb_results,
                   vector<SpaceResult<ID>>* results) const;

    size_t Size() const;

    void Info(FILE* log, size_t indent=2, size_t indent_incr=4) const;

  private:
    size_t nb_dims_;
    unordered_map<ID, size_t> id2index_;
    vector<ID> ids_;
    vector<float> point_floats_;
};


template <typename ID>
LinearSpace<ID>::LinearSpace() {
}

template <typename ID>
LinearSpace<ID>::~LinearSpace() {
}

template <typename ID>
void LinearSpace<ID>::Init(size_t nb_dims) {
    nb_dims_ = nb_dims;
    id2index_.clear();
    ids_.clear();
    point_floats_.clear();
}

template <typename ID>
void LinearSpace<ID>::Clear() {
    id2index_.clear();
    ids_.clear();
    point_floats_.clear();
}

template <typename ID>
unsigned int LinearSpace<ID>::Delete(const ID& id) {
    // If we have nothing, no lookup needed.
    if (ids_.empty()) {
        return 0;
    }

    // Look up the ID.
    auto it = id2index_.find(id);
    if (it == id2index_.end()) {
        return 0;
    }

    // Swap with the end and resize by one.
    auto& index = it->second;
    id2index_[ids_[ids_.size() - 1]] = index;
    ids_[index] = ids_[ids_.size() - 1];
    ids_.resize(ids_.size() - 1);
    for (size_t i = 0; i < nb_dims_; ++i) {
        size_t to_index = index * nb_dims_ + i;
        size_t from_index = ids_.size() * nb_dims_ + i;
        point_floats_[to_index] = point_floats_[from_index];
    }
    point_floats_.resize(ids_.size() * nb_dims_);
	return 1;
}

template <typename ID>
unsigned int LinearSpace<ID>::Upsert(const SpaceInput<ID>& input) {
    Delete(input.id);

    // Reject NaN entries.
    if (isnan(input.point[0])) {
        return 0;
    }

    ids_.emplace_back(input.id);

    for (size_t i = 0; i < nb_dims_; ++i) {
        point_floats_.emplace_back(input.point[i]);
    }

	return 1;
}

/*
template <typename ID>
void LinearSpace<ID>::GetNeighbors(const float* point, size_t nb_results,
                                vector<SpaceResult<ID>>* results) const {
    results->clear();
    results->reserve(ids_.size());
    for (size_t i = 0; i < ids_.size(); ++i) {
        SpaceResult<ID> r;
        r.id = ids_[i];
        const float* other_point = &point_floats_[i * nb_dims_];
        r.dist = EuclideanDistance(point, other_point, nb_dims_);
        results->emplace_back(r);
    }
    sort(results->begin(), results->end());
    if (nb_results < results->size()) {
        results->resize(nb_results);
    }
}
*/

namespace {

template <typename ID>
struct LinearSpaceThreadData {
    size_t id;
    size_t nb_threads;

    const float* point;
    size_t nb_results;

    size_t nb_dims;
    const vector<ID>* ids;
    const vector<float>* point_floats;

    vector<SpaceResult<ID>>* results;
};

template <typename ID>
void* NeighborsSortMT(void* arg) {
    LinearSpaceThreadData<ID>* data = (LinearSpaceThreadData<ID>*)arg;
    data->results->clear();
    size_t begin = data->id * data->ids->size() / data->nb_threads;
    size_t end = (data->id + 1) * data->ids->size() / data->nb_threads;
    data->results->reserve(end - begin);
    for (size_t i = begin; i < end; ++i) {
        SpaceResult<ID> r;
        r.id = (*data->ids)[i];
        const float* other_point = &(*(data->point_floats))[i * data->nb_dims];
        r.dist = EuclideanDistance(data->point, other_point, data->nb_dims);
        data->results->emplace_back(r);
    }
    sort(data->results->begin(), data->results->end());
    if (data->nb_results < data->results->size()) {
        data->results->resize(data->nb_results);
    }
    pthread_exit(nullptr);
}

template <typename ID>
void* NeighborsKBestSetMT(void* arg) {
    LinearSpaceThreadData<ID>* data = (LinearSpaceThreadData<ID>*)arg;
    data->results->clear();
    size_t begin = data->id * data->ids->size() / data->nb_threads;
    size_t end = (data->id + 1) * data->ids->size() / data->nb_threads;

    set<SpaceResult<ID>> best;
    for (size_t i = begin; i < end; ++i) {
        SpaceResult<ID> r;
        r.id = (*data->ids)[i];
        const float* other_point = &(*(data->point_floats))[i * data->nb_dims];
        r.dist = EuclideanDistance(data->point, other_point, data->nb_dims);
        if (best.size() < data->nb_results) {
            best.insert(r);
        } else {
            auto it = best.rbegin();
            auto& current_biggest = *it;
            if (r < current_biggest) {
                best.erase(current_biggest);
                best.insert(r);
            }
        }
    }

    size_t i = 0;
    for (auto it : best) {
        if (i == data->nb_results) {
            break;
        }
        data->results->emplace_back(it);
    }
    pthread_exit(nullptr);
}

template <typename ID>
void* NeighborsKBestVectorMT(void* arg) {
    LinearSpaceThreadData<ID>* data = (LinearSpaceThreadData<ID>*)arg;
    data->results->clear();
    size_t begin = data->id * data->ids->size() / data->nb_threads;
    size_t end = (data->id + 1) * data->ids->size() / data->nb_threads;

    vector<SpaceResult<ID>> bests;
    bests.reserve(data->nb_results);
    for (size_t i = begin; i < end; ++i) {
        SpaceResult<ID> r;
        r.id = (*data->ids)[i];
        const float* other_point = &(*(data->point_floats))[i * data->nb_dims];
        r.dist = EuclideanDistance(data->point, other_point, data->nb_dims);
        if (bests.size() < data->nb_results) {
            bests.emplace_back(r);
            sort(bests.begin(), bests.end());
        } else {
            auto& current_biggest = bests[bests.size() - 1];
            if (r.dist < current_biggest.dist) {
                current_biggest = r;
                sort(bests.begin(), bests.end());
            }
        }
    }

    size_t i = 0;
    for (auto it : bests) {
        if (i == data->nb_results) {
            break;
        }
        data->results->emplace_back(it);
    }
    pthread_exit(nullptr);
}

}  // namespace

template <typename ID>
void LinearSpace<ID>::GetNeighbors(const float* point, size_t nb_results,
                                vector<SpaceResult<ID>>* results) const {
    results->clear();

    size_t nb_threads = std::thread::hardware_concurrency();

    vector<pthread_t> threads;
    threads.resize(nb_threads);
    vector<LinearSpaceThreadData<ID>> thread_data;
    thread_data.resize(nb_threads);
    vector<vector<SpaceResult<ID>>> results_per_thread;
    results_per_thread.resize(nb_threads);
    for (size_t i = 0; i < nb_threads; ++i) {
        auto& info = thread_data[i];
        info.id = i;
        info.nb_threads = nb_threads;
        info.point = point;
        info.nb_results = nb_results;
        info.nb_dims = nb_dims_;
        info.ids = &ids_;
        info.point_floats = &point_floats_;
        info.results = &results_per_thread[i];
        pthread_create(&threads[i], nullptr, NeighborsKBestVectorMT<ID>, &info);
    }

    for (size_t i = 0; i < nb_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    for (auto& sub_results : results_per_thread) {
        for (auto& r : sub_results) {
            results->emplace_back(r);
        }
    }
    sort(results->begin(), results->end());
    if (nb_results < results->size()) {
        results->resize(nb_results);
    }
}

template <typename ID>
size_t LinearSpace<ID>::Size() const {
    return ids_.size();
}

template <typename ID>
void LinearSpace<ID>::Info(FILE* log, size_t indent, size_t indent_incr) const {
    const char* zero = string(indent, ' ').c_str();
    fprintf(log, "%sitems: %zu\n", zero, ids_.size());
}
