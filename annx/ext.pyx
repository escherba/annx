__all__ = ["ANNX"]

import numpy as np
cimport numpy as np
cimport cython

cimport gauss_lsh
from gauss_lsh cimport LSHSpace, uint32_t

np.import_array()

cdef class ANNX(object):

    cdef LSHSpace[uint32_t]* _indexer

    def __cinit__(self):
        pass

    def __init__(self, L, k, rank, w=0.5, normalize_inputs=False):
        self._indexer = new LSHSpace[uint32_t](L, k, rank, 0)
