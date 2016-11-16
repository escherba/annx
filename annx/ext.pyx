# cython: boundscheck=False
# cython: wraparound=False
# cython: nonecheck=False
# cython: infer_types=True

__all__ = ["ANNX"]

import numpy as np
cimport numpy as np
cimport cython

from libc.stdint cimport uint32_t, uint64_t
from libcpp.vector cimport vector

cimport gauss_lsh
from gauss_lsh cimport LSHSpace, SpaceInput, SpaceResult

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

    def size(self):
        return self._indexer.Size()

    def upsert(self, uint64_t id, np.ndarray[np.float32_t, ndim=1, mode='c'] arr not None):
        assert(len(arr) == self._rank)
        cdef SpaceInput[uint64_t] input
        input.id = id
        input.point = <const float*>arr.data
        return self._indexer.Upsert(input)

    cdef _query_result(self, vector[SpaceResult[uint64_t]] vec):
        cdef np.ndarray[np.uint64_t, ndim=1] ids = \
            np.ndarray(shape=(vec.size(),), dtype=np.uint64)
        cdef np.ndarray[np.float32_t, ndim=1] distances = \
            np.ndarray(shape=(vec.size(),), dtype=np.float32)
        cdef size_t i;
        for i in xrange(vec.size()):
            ids[i] = vec[i].id
            distances[i] = vec[i].dist
        return (ids, distances)

    def query_id(self, uint64_t id, uint32_t n_neighbors=10):
        cdef vector[SpaceResult[uint64_t]] vec
        vec.reserve(n_neighbors)
        self._indexer.GetNeighborsById(id, n_neighbors, vec)
        return self._query_result(vec)

    def query_point(self, np.ndarray[np.float32_t, ndim=1, mode='c'] arr not None, uint32_t n_neighbors=10):
        cdef vector[SpaceResult[uint64_t]] vec
        vec.reserve(n_neighbors)
        self._indexer.GetNeighborsByPt(<const float*>arr.data, n_neighbors, vec)
        return self._query_result(vec)

    def __dealloc__(self):
        del self._indexer
