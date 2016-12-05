"""
code after http://stackoverflow.com/a/10840981/597371
"""

from random import randrange
from collections import MutableMapping


class RandomChoiceDict(MutableMapping):
    def __init__(self):
        self._mapping = {}  # wraps a _mapping
        self._id2key = {}  # e.g. {0:'a', 1:'c' 2:'b'},
        self._key2id = {}  # needed to help delete elements

    def __getitem__(self, key):  # O(1)
        return self._mapping[key]

    def __len__(self):
        return len(self._mapping)

    def __iter__(self):
        return iter(self._mapping)

    def __setitem__(self, key, value):  # O(1)
        mapping = self._mapping
        if key in mapping:
            mapping[key] = value
        else:  # new item
            idx = len(mapping)
            mapping[key] = value

            # add it to the arbitrary bijection
            self._id2key[idx] = key
            self._key2id[key] = idx

    def __delitem__(self, key):  # O(1)
        mapping = self._mapping
        key2id = self._key2id
        id2key = self._id2key
        del mapping[key]  # O(1) average case

        emptyIdx = key2id[key]
        largestIdx = len(mapping)  # about to be deleted
        largestIdxKey = id2key[largestIdx]  # going to store this in empty Idx

        # swap deleted element with highest-id element in arbitrary map:
        id2key[emptyIdx] = largestIdxKey
        key2id[largestIdxKey] = emptyIdx

        del key2id[key]
        del id2key[largestIdx]

    def random_key(self):
        return self._id2key[randrange(len(self._mapping))]

    def random_item(self):  # O(1)
        key = self.random_key()
        return (key, self._mapping[key])

    def random_value(self):
        return self._mapping[self.random_key()]

    def sample_keys(self, n, replace=False):
        """sample - returns generator over random sample

        :param n: sample size
        :param replace: sample with replacement (default: False)

        (This method gets slow once n becomes large)
        """
        total = len(self._mapping)
        seen = set()
        returned = 0
        while returned < n:
            key = self._id2key[randrange(total)]
            if replace or key not in seen:
                returned += 1
                seen.add(key)
                yield key

    def sample_values(self, n, replace=False):
        mapping = self._mapping
        return [mapping[key] for key in self.sample_keys(n, replace=replace)]

    def sample_items(self, n, replace=False):
        mapping = self._mapping
        return [(key, mapping[key]) for key in self.sample_keys(n, replace=replace)]
