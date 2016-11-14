cimport cython

cdef extern from * nogil:
    ctypedef float float32_t
    ctypedef unsigned long int uint32_t
    ctypedef unsigned long long int uint64_t


cdef extern from "annx/space.h" nogil:
    cdef struct SpaceInput[T]:
        T id
        const float32_t* dist
