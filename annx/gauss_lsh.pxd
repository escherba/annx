cimport numpy as np
from libcpp.string cimport string
from libcpp.vector cimport vector

cimport cython

cdef extern from * nogil:
    ctypedef unsigned long int uint32_t
    ctypedef unsigned long long int uint64_t

cdef extern from "ann/space.h" nogil:
    cdef cppclass SpaceInput[T]:
        T id
        const float* point

    cdef cppclass SpaceResult[T]:
        T id
        float dist
        bint operator <  (SpaceResult&, SpaceResult&)
        bint operator == (SpaceResult&, SpaceResult&)


cdef extern from "ann/gauss_lsh.h" nogil:
    cdef cppclass LSHSpace[T]:
        LSHSpace(size_t L, size_t k, float w, uint64_t seed)
        void Init(size_t nb_dims)
        void Clear()
        uint32_t Delete(const T& id)
        uint32_t Upsert(const SpaceInput[T]& input)
        void GetNeighborsById "GetNeighbors" (const T& id, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        void GetNeighborsByPt "GetNeighbors" (const float* point, size_t nb_results,
                          vector[SpaceResult[T]]& results)
        size_t Size()
