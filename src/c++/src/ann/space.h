#pragma once

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>


using std::string;
using std::vector;

template <typename ID>
struct SpaceInput {
    ID id;
    const float* point;
};

/* begin SpaceResult<ID> */
template <typename ID>
struct SpaceResult {
    ID id;
    float dist;
    bool operator<(const SpaceResult& o) const;
    bool operator==(const SpaceResult& o) const;
};

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
/* end SpaceResult<ID> */

/* beghin Space<ID> */
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

    virtual void GraphToStream(std::ostream& out, size_t nb_results) const = 0;

    virtual void GraphToPath(const std::string& path, size_t nb_results) const;

    // Get the number of elements stored.
    virtual size_t Size() const = 0;

    // Get Dimensionality
    virtual size_t Dim() const = 0;

    // Dump statistics about internals.
    virtual void Info(FILE* log, size_t indent=2,
                      size_t indent_incr=4) const;
};

template <typename ID>
void Space<ID>::Info(FILE* log, size_t indent, size_t indent_incr) const {
    const char* zero = string(indent, ' ').c_str();
    fprintf(log, "%sitems: %zu\n", zero, Size());
}

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
void Space<ID>::GraphToPath(const std::string& path, size_t nb_results) const {
    if (path == "-") {
        GraphToStream(std::cout, nb_results);
    } else {
        std::ofstream ofs;
        ofs.open(path, std::ofstream::out | std::ofstream::trunc);
        GraphToStream(ofs, nb_results);
        ofs.close();
    }
}

/* end Space<ID> */

template <typename ID>
inline void WriteResults(std::ostream& out, const ID& id, const vector<SpaceResult<ID>>& results) {
    for (auto& result: results) {
        out << id << "," << result.id << "," << result.dist << std::endl;
    }
}

template <typename Float>
inline Float EuclideanDistance(const Float* a, const Float* b, size_t dim) {
    Float result = 0.0;
    #pragma omp simd reduction(+:result) aligned(a:32)
    for (size_t i = 0; i < dim; ++i) {
        result += a[i] * b[i];
    }
    return (Float)sqrt(2.0 * std::max(0.0f, 1.0 - result));
}

template <typename Float>
inline Float CosineDistance(const Float* a, const Float* b, size_t dim) {
    Float result = 0.0;
    #pragma omp simd reduction(+:result) aligned(a:32)
    for (size_t i = 0; i < dim; ++i) {
        result += a[i] * b[i];
    }
    return (Float)std::max(0.0f, 1.0 - result);
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
inline Float norm(const Float* a, size_t dim) {
    Float result = 0.0;
    #pragma omp simd reduction(+:result)
    for (size_t i = 0; i < dim; ++i) {
        Float el = a[i];
        result += el * el;
    }
    return (Float)sqrt(result);
}

template <typename Float>
bool normalize(Float* dst, const Float* src, size_t dim) {
    Float factor = 1.0 / norm(src, dim);
    if (!std::isfinite(factor)) {
        return false;
    }
    #pragma omp simd linear(src,dst)
    for (size_t i = 0; i < dim; ++i) {
        *dst = *src * factor;
        src++; dst++;
    }
    return true;
}

// ----------------------------

typedef uint32_t AnyID;
void MakeSpace(const string& space_algo, Space<AnyID>** space, size_t nb_dims);
