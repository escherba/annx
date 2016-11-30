# cython: boundscheck=False
# cython: wraparound=False
# cython: nonecheck=False
# cython: infer_types=True

__all__ = ["LinearIndexer", "LSHIndexer"]

import numpy as np
cimport numpy as np
cimport cython

from libc.stdint cimport uint32_t, uint64_t
from libcpp.string cimport string
from libcpp.vector cimport vector

cimport space
from space cimport Space, LinearSpace, LSHSpace, SpaceInput, SpaceResult

np.import_array()


cdef class Indexer:

    cdef Space[uint64_t]* _indexer
    cdef uint32_t _rank

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

    cdef tuple _query_result(self, vector[SpaceResult[uint64_t]] results):
        cdef size_t sz = results.size()
        cdef np.ndarray[np.uint64_t, ndim=1] ids = \
            np.empty(shape=(sz,), dtype=np.uint64)
        cdef np.ndarray[np.float32_t, ndim=1] distances = \
            np.empty(shape=(sz,), dtype=np.float32)
        cdef size_t i = 0;
        cdef SpaceResult[uint64_t] result
        for result in results:
            ids[i] = result.id
            distances[i] = result.dist
            i += 1
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


cdef class LSHIndexer(Indexer):

    def __cinit__(self, uint32_t rank, uint32_t L=15, uint32_t k=32, float w=0.5, uint32_t search_k=0, uint64_t seed=0):
        self._indexer = new LSHSpace[uint64_t](seed)
        (<LSHSpace[uint64_t] *>self._indexer).Config(rank, L, k, w, search_k)
        self._rank = rank

    def __dealloc__(self):
        del self._indexer


cdef class LinearIndexer(Indexer):

    def __cinit__(self, uint32_t rank):
        self._indexer = new LinearSpace[uint64_t]()
        self._indexer.Init(rank)
        self._rank = rank

    def __dealloc__(self):
        del self._indexer
