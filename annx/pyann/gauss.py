import numpy as np
import logging
from collections import defaultdict, Counter
from itertools import izip, imap
from operator import itemgetter
from tqdm import tqdm
from cityhash import CityHash32
from .containers import RandomChoiceDict


class NDHashable(object):
    """Turn NumPy NDArray into a hashable object

    A more sparse alternative to the default Python hash() method.
    """
    def __init__(self, arr):
        self.data = arr.tostring()

    def __eq__(self, other):
        return self.data == other.data

    def __hash__(self):
        return CityHash32(self.data)


class GaussianLSH(object):
    """Locality-sensitive hashing for nearest neighbor lookups

    Designed with updatable indexes in mind. Assumes that the input vectors
    are already normalized.

    The implementation is based on:

    Malcolm Slaney, Yury Lifshits, and Junfeng He "Optimal Parameters for
    Locality-Sensitive Hashing" Proceedings of the IEEE, vol.100, no.9,
    pp.2604-2623, September 2012
    doi: 10.1109/JPROC.2012.2193849
    URL: http://dx.doi.org/10.1109/JPROC.2012.2193849

    """
    def __init__(self, rank, L=15, k=32, w=0.5, normalize_inputs=False,
                 dtype=np.float32):
        """Default constructor

        :param L: number of tables
        :param k: bit depth for within-table lookups
        :param rank: rank (size) of the input vectors
        :param w: quantization window parameter (default: 0.5)
        :param normalize_inputs: will bring inputs to L2 norm before indexing
        """
        self.w_ = w
        self.dtype = dtype
        self.gauss_ = self._create_gauss(L, k, rank, dtype=self.dtype)
        self.shift_ = self._create_shift(L, k, w, dtype=self.dtype)
        self.tables = [defaultdict(set) for _ in xrange(L)]
        self.vectors = RandomChoiceDict()
        self._second = itemgetter(1)
        self._similarity = np.dot
        self._normalize = normalize_inputs

    @staticmethod
    def _create_gauss(L, k, rank, dtype=np.float32):
        """Generate Gaussian projection vectors

        :param L: number of tables
        :param k: bit depth for within-table lookups
        :param rank: rank (size) of the input vectors
        """
        vec = np.asarray(np.random.normal(size=(L, k, rank)), dtype=dtype)
        norm = np.linalg.norm(vec, axis=-1)
        vec /= norm.reshape(L, k, 1)
        return vec

    @staticmethod
    def _create_shift(L, k, w=0.5, dtype=np.float32):
        """Generate shift parameter `b`

        :param L: number of tables
        :param k: bit depth for within-table lookups
        :param w: quantization window parameter (default: 0.5)
        """
        return np.asarray(np.random.uniform(size=(L, k)), dtype=dtype) * w

    def _iter_hashes(self, vector):
        """iterate over hashes

        :param vector: vector (NumPy array)
        """
        projection = self.gauss_.dot(vector)
        quantized = np.floor((projection + self.shift_) /
                             self.w_).astype(np.int8)
        keys = imap(NDHashable, quantized)
        return izip(self.tables, keys)

    def insert_rows(self, rows, total=None):
        """Insert iterrows() result from [label, vector] table

        :param rows: Pandas dataframe iterrows() result
        :param total: expected number of rows
        """
        n_skipped = 0
        for _, (label, vector) in tqdm(rows, total=total):
            if self.insert(label, vector) == 0:
                n_skipped += 1
        if n_skipped > 0:
            logging.warn("%d items skipped", n_skipped)

    def insert(self, label, vector, update=False):
        """Insert item

        :param label: label belonging to the inserted item
        :param vector: vector belonging to the inserted item
        :param update: if true, remove indices belonging to existing
                       item if one exists and shares the same label
        """
        if update:
            self.remove(label, indices_only=True)
        vector = np.asarray(vector, self.dtype)
        if self._normalize:
            vector = vector / np.linalg.norm(vector)
            if np.isnan(vector).any():
                return 0
        self.vectors[label] = vector
        for table, key in self._iter_hashes(vector):
            table[key].add(label)
        return 1

    def upsert(self, label, vector):
        """Update vector (inserts new item if not found)

        :param label: label of the item to update
        :param vector: vector to update the label to
        """
        self.insert(label, vector, update=True)

    def remove(self, label, indices_only=False):
        """Remove item and associated indices

        :param label: label of the item to remove
        :param indices_only: if True, will not erase value (vector)
        """
        vectors = self.vectors
        if label in vectors:
            old_vector = vectors[label]
            for table, key in self._iter_hashes(old_vector):
                if key in table:
                    labels = table[key]
                    labels.remove(label)
                    if len(labels) == 0:
                        del table[key]
            if not indices_only:
                del vectors[label]

    def _candidates(self, vector, search_k=None, drop_labels=()):
        """Generate candidates for further analysis

        :param vector: vector to lookup (NumPy array of floats)
        :param search_k: maximum number of candidates to generate
        :param drop_labels: list of labels to discard from query
        """
        c = Counter()
        vectors = self.vectors
        for table, key in self._iter_hashes(vector):
            if key in table:
                c.update(table[key])
        for label in drop_labels:
            if label in c:
                del c[label]
        return [label for label, _ in c.most_common(search_k)
                if label in vectors]

    def query_vector(self, vector, limit=None, drop_labels=(),
                     ensure_limit=False, search_k=None, _allow_norm=True):
        """Query a vector

        :param vector: vector to lookup (NumPy array of floats)
        :param limit: number of neighbors to evaluate
        :param drop_labels: list of labels to discard from query
        :param ensure_limit: If True, randomly sample neighbors until limit
        :param search_k: how many candidates to search
        """
        if search_k is None and limit is not None:
            search_k = limit * len(self.tables)
        if _allow_norm and self._normalize:
            vector = np.asarray(vector, self.dtype)
            vector = vector / np.linalg.norm(vector)
            if np.isnan(vector).any():
                logging.warn("NaNs present in input")
                return []
        labels = self._candidates(
            vector, search_k, drop_labels=drop_labels)
        vectors = self.vectors
        if ensure_limit:
            n = limit - len(labels)
            for label in vectors.sample_keys(n, replace=False):
                if label not in drop_labels:
                    labels.append(label)
        cand_vectors = np.array([vectors[label] for label in labels])
        similarities = self._similarity(cand_vectors, vector)
        return sorted(izip(labels, similarities),
                      key=self._second, reverse=True)[:limit]

    def query_label(self, label, limit=None, drop_self=False,
                    ensure_limit=False, search_k=None):
        """Query a label for an item already in the index

        :param label: label to lookup
        :param limit: number of neighbors to return
        :param drop_self: whether to drop query label from results
        :param ensure_limit: if True, randomly sample neighbors until limit
        :param search_k: how many candidates to search
        """
        vector = self.vectors[label]
        drop_labels = [label] if drop_self else []
        return self.query_vector(
            vector, limit=limit, drop_labels=drop_labels,
            ensure_limit=ensure_limit, search_k=search_k, _allow_norm=False)
