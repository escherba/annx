import unittest
import numpy as np
import annx


class TestLSHIndexer(unittest.TestCase):

    def test_query_point(self):
        """query_point method should work
        """
        indexer = annx.LSHIndexer(10)
        vec = np.random.random(size=(10,)).astype(np.float32)
        indexer.upsert(1, vec)
        indexer.upsert(2, vec)
        res = indexer.query_point(vec)
        self.assertEqual(len(res), 2)
        self.assertSetEqual(set(res[0]), set([1, 2]))

    def test_query_id(self):
        """query_id method should work
        """
        indexer = annx.LSHIndexer(10)
        vec = np.random.random(size=(10,)).astype(np.float32)
        indexer.upsert(1, vec)
        indexer.upsert(2, vec)
        res = indexer.query_id(1)
        self.assertEqual(len(res), 2)
        self.assertSetEqual(set(res[0]), set([1, 2]))


class TestLinearIndexer(unittest.TestCase):

    def test_query_point(self):
        """query_point method should work
        """
        indexer = annx.LinearIndexer(10)
        vec = np.random.random(size=(10,)).astype(np.float32)
        indexer.upsert(1, vec)
        indexer.upsert(2, vec)
        res = indexer.query_point(vec)
        self.assertEqual(len(res), 2)
        self.assertSetEqual(set(res[0]), set([1, 2]))

    def test_query_id(self):
        """query_id method should work
        """
        indexer = annx.LinearIndexer(10)
        vec = np.random.random(size=(10,)).astype(np.float32)
        indexer.upsert(1, vec)
        indexer.upsert(2, vec)
        res = indexer.query_id(1)
        self.assertEqual(len(res), 2)
        self.assertSetEqual(set(res[0]), set([1, 2]))
