cimport numpy as np
from libc.stdint cimport uint32_t, uint64_t
from libcpp.string cimport string
from libcpp.vector cimport vector

cimport cython


cdef extern from "ann/space.h" nogil:
    cdef cppclass SpaceInput[T]:
        T id
        const float* point

    cdef cppclass SpaceResult[T]:
        T id
        float dist
        bint operator <  (SpaceResult&, SpaceResult&)
        bint operator == (SpaceResult&, SpaceResult&)


cdef extern from "ann/space.h" nogil:
    cdef cppclass Space[T]:
        Space()
        void Init(size_t nb_dims)
        void Clear()
        uint32_t Delete(const T& id)
        uint32_t Upsert(const SpaceInput[T]& input)
        void GetNeighborsById "GetNeighbors" (const T& id, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GetNeighborsByPt "GetNeighbors" (const float* point, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GraphToPath(const string& path, size_t nb_results)
        size_t Size()


cdef extern from "ann/gauss_lsh.h" nogil:
    cdef cppclass LSHSpace[T](Space[T]):
        LSHSpace(uint64_t seed)
        void Config(size_t nb_dims, size_t L, size_t k, float w, size_t search_k)
        void Init(size_t nb_dims)
        void Clear()
        uint32_t Delete(const T& id)
        uint32_t Upsert(const SpaceInput[T]& input)
        void GetNeighborsById "GetNeighbors" (const T& id, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GetNeighborsByPt "GetNeighbors" (const float* point, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GraphToPath(const string& path, size_t nb_results)
        size_t Size()


cdef extern from "ann/linear_space.h" nogil:
    cdef cppclass LinearSpace[T](Space[T]):
        LinearSpace()
        void Init(size_t nb_dims)
        void Clear()
        uint32_t Delete(const T& id)
        uint32_t Upsert(const SpaceInput[T]& input)
        void GetNeighborsById "GetNeighbors" (const T& id, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GetNeighborsByPt "GetNeighbors" (const float* point, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GraphToPath(const string& path, size_t nb_results)
        size_t Size()
