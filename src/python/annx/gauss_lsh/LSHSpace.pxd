cimport numpy as np
from libcpp.string cimport string
from libcpp.vector cimport vector

cimport cython

cdef extern from * nogil:
    ctypedef float float32_t
    ctypedef unsigned long int uint32_t
    ctypedef unsigned long long int uint64_t


cdef extern from "annx/gauss_lsh.h" nogil:
    cdef cppclass LSHSpace[T]:
        LSHSpace(size_t L=15, size_t k=32, float32_t w=0.5, uint64_t seed=0)
        void Init(size_t nb_dims)
        void Clear()
        uint32_t Delete(const T& id)
        uint32_t Upsert(const SpaceInput[T]& input)
        void GetNeighbors(const T& id, size_t nb_results,
                          vector[SpaceResult[T]]* results)
        size_t Size()
