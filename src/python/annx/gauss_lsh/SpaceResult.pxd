cimport cython

cdef extern from * nogil:
    ctypedef float float32_t
    ctypedef unsigned long int uint32_t
    ctypedef unsigned long long int uint64_t


cdef extern from "annx/space.h" nogil:
    cdef struct SpaceResult[T]:
        T id
        float32_t dist
        bint operator <  (SpaceResult&, SpaceResult&)
        bint operator == (SpaceResult&, SpaceResult&)
