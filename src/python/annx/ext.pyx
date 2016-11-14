__all__ = ["ANNX"]

import numpy as np
cimport numpy as np
cimport cython

from .gauss_lsh cimport LSHSpace
from .gauss_lsh cimport SpaceInput
from .gauss_lsh cimport SpaceResult

np.import_array()

cdef class ANNX(object):

    cdef object _indexer

    def __cinit__(self, L, k, rank, w=0.5, normalize_inputs=False):
        self._indexer = LSHSpace(L, k, rank, w, normalize_inputs)

    def __init__(self):
        pass
