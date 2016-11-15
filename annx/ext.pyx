__all__ = ["ANNX"]

import numpy as np
cimport numpy as np
cimport cython

cimport gauss_lsh
from gauss_lsh cimport LSHSpace, SpaceInput, SpaceResult, \
    uint32_t, uint64_t, vector

np.import_array()


cdef class ANNX(object):

    cdef LSHSpace[uint64_t]* _indexer
    cdef uint32_t _rank

    def __cinit__(self):
        pass

    def __init__(self, uint32_t rank, L=15, k=32, w=0.5, normalize_inputs=False):
        self._indexer = new LSHSpace[uint64_t](L, k, rank, 0)
        self._indexer.Init(rank)
        self._rank = rank

    def remove(self, uint64_t id):
        self._indexer.Delete(id)

    def clear(self):
        self._indexer.Clear()

    def upsert(self, uint64_t id, np.ndarray[np.float32_t, ndim=1, mode='c'] arr):
        assert(len(arr) == self._rank)
        cdef SpaceInput[uint64_t] input
        input.id = id
        input.point = <const float*>arr.data
        self._indexer.Upsert(input)

    def neighbors_by_id(self, uint64_t id, uint32_t n):
        cdef vector[SpaceResult[uint64_t]] vec
        vec.reserve(n)
        self._indexer.GetNeighborsById(id, n, vec)

    def neighbors_by_vec(self, np.ndarray[np.float32_t, ndim=1, mode='c'] arr, int n):
        pass

    def __dealloc__(self):
        del self._indexer
