#pragma once

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <string>


using std::string;
using std::vector;
using std::isfinite;

template <typename ID>
struct SpaceInput {
    ID id;
    const float* point;
};

template <typename ID>
struct SpaceResult {
    ID id;
    float dist;
    bool operator<(const SpaceResult& o) const;
    bool operator==(const SpaceResult& o) const;
};

template <typename ID>
class Space {
  public:
    Space() {}
    virtual ~Space() {}

    // Initialize.  You must call this before use.
    virtual void Init(size_t embed_dim) = 0;

    // Remove all elements.
    virtual void Clear() = 0;

    // Remove an ID.
    virtual unsigned int Delete(const ID& id) = 0;
    virtual unsigned int DeleteMany(const vector<ID>& ids);

    // Remove the ID if it exists, then insert it at the given point.
    virtual unsigned int Upsert(const SpaceInput<ID>& input) = 0;
    virtual unsigned int UpsertMany(const vector<SpaceInput<ID>>& inputs);

    // Get the nearest neighbors of a point.
    virtual void GetNeighbors(const float* point, size_t nb_results,
            vector<SpaceResult<ID>>& results) const = 0;

    virtual void GetNeighbors(const ID& id, size_t nb_results,
            vector<SpaceResult<ID>>& results) const = 0;

    // Get the number of elements stored.
    virtual size_t Size() const = 0;

    // Dump statistics about internals.
    virtual void Info(FILE* log, size_t indent=2,
                      size_t indent_incr=4) const = 0;
};

template <typename ID>
unsigned int Space<ID>::DeleteMany(const vector<ID>& ids) {
    int count = 0;
    for (auto& id : ids) {
        count += Delete(id);
    }
    return count;
}

template <typename ID>
unsigned int Space<ID>::UpsertMany(const vector<SpaceInput<ID>>& inputs) {
    int count = 0;
    for (auto& input : inputs) {
        count += Upsert(input);
    }
    return count;
}

template <typename ID>
bool SpaceResult<ID>::operator<(const SpaceResult<ID>& o) const {
    if (dist == o.dist) {
        return id < o.id;
    }

    return dist < o.dist;
}

template <typename ID>
bool SpaceResult<ID>::operator==(const SpaceResult<ID>& o) const {
    return id == o.id && dist == o.dist;
}

template <typename Float>
Float EuclideanDistance(const Float* a, const Float* b, size_t dim) {
    Float result = 0.0;
    #pragma omp simd reduction(+:result)
    for (size_t i = 0; i < dim; ++i) {
        result += (a[i] - b[i]) * (a[i] - b[i]);
    }
    return (Float)sqrt(result);
}

template <typename Float>
Float CosineDistance(const Float* a, const Float* b, size_t dim) {
    Float result = 0.0;
    #pragma omp simd reduction(+:result)
    for (size_t i = 0; i < dim; ++i) {
        result += a[i] * b[i];
    }
    return (Float)(1.0 - result);
}

template <typename Float>
inline bool isfinite_xf(const Float* arr, size_t dim) {
    const Float* end = arr + dim;
    for (; arr < end; ++arr) {
        if (!std::isfinite(*arr)) {
            return false;
        }
    }
    return true;
}

template <typename Float>
inline Float norm(const Float* arr, size_t dim) {
    Float result = 0.0;
    const Float *end = arr + dim;
    #pragma omp simd reduction(+:result)
    for (size_t i = 0; i < dim; ++i) {
        Float el = arr[i];
        result += el * el;
    }
    return (Float)sqrt(result);
}

// ----------------------------

typedef uint32_t AnyID;
void MakeSpace(const string& space_algo, Space<AnyID>** space, size_t nb_dims);
