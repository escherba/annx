# cython: boundscheck=False
# cython: wraparound=False
# cython: nonecheck=False
# cython: infer_types=True

__all__ = ["ANNX"]

import numpy as np
cimport numpy as np
cimport cython

from libc.stdint cimport uint32_t, uint64_t
from libcpp.string cimport string
from libcpp.vector cimport vector

cimport gauss_lsh
from gauss_lsh cimport LSHSpace, SpaceInput, SpaceResult

np.import_array()


cdef class ANNX:

    cdef LSHSpace[uint64_t]* _indexer
    cdef uint32_t _rank

    def __cinit__(self, uint32_t rank, L=15, k=32, w=0.5, search_k=0, seed=0):
        self._indexer = new LSHSpace[uint64_t](seed)
        self._indexer.Config(rank, L, k, w, search_k)
        self._rank = rank

    def remove(self, uint64_t id):
        self._indexer.Delete(id)

    def clear(self):
        self._indexer.Clear()

    def size(self):
        return self._indexer.Size()

    def upsert(self, uint64_t id,
               np.ndarray[np.float32_t, ndim=1, mode='c'] vec not None):
        assert(len(vec) == self._rank)
        cdef SpaceInput[uint64_t] input
        input.id = id
        input.point = <const float*>vec.data
        return self._indexer.Upsert(input)

    cdef _query_result(self, vector[SpaceResult[uint64_t]] results):
        cdef np.ndarray[np.uint64_t, ndim=1] ids = \
            np.ndarray(shape=(results.size(),), dtype=np.uint64)
        cdef np.ndarray[np.float32_t, ndim=1] distances = \
            np.ndarray(shape=(results.size(),), dtype=np.float32)
        cdef size_t i;
        for i in xrange(results.size()):
            ids[i] = results[i].id
            distances[i] = results[i].dist
        return (ids, distances)

    def query_id(self, uint64_t id, uint32_t n_neighbors=10):
        cdef vector[SpaceResult[uint64_t]] results
        self._indexer.GetNeighborsById(id, n_neighbors, results)
        return self._query_result(results)

    def query_point(self, np.ndarray[np.float32_t, ndim=1, mode='c'] vec not None,
                    uint32_t n_neighbors=10):
        cdef vector[SpaceResult[uint64_t]] results
        self._indexer.GetNeighborsByPt(<const float*>vec.data, n_neighbors, results)
        return self._query_result(results)

    def make_graph(self, output, uint32_t nb_neighbors=10):
        cdef string path
        if isinstance(output, basestring):
            path = output.encode("utf-8")
            self._indexer.MakeGraph(path, nb_neighbors)
        else:
            raise TypeError(output)

    def __dealloc__(self):
        del self._indexer
