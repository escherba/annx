import unittest
import re
import numpy as np
from annx.pyann.gauss import GaussianLSH


def mat_from_text(text):
    lines = text.strip().split("\n")
    lines = [re.split("\\s+", line.strip()) for line in lines]
    data = [map(float, line) for line in lines]
    data = np.array(data)
    return data


class TestLSH(unittest.TestCase):

    def test_case1(self):
        mat = mat_from_text("""
  0.188792  0.0254653  -0.150737  -0.237212 0.00273913     0.3225  -0.661775   0.197832   0.218752   0.505735
 -0.173927  -0.145668   0.258059  -0.479877   0.150525   0.269261  -0.512162 -0.0997509  0.0130044   0.532977
 0.0031375 -0.0397231  -0.303223  0.0397919  -0.352977   0.212393  -0.454782  -0.538179  -0.362703   0.327359
        """)
        vec = mat_from_text("""
   0.0042174   -0.0134243   0.00527203 -0.000760696  -0.00267559    -0.015297   -0.0311168   0.00506558   -0.0193664   0.00619034
        """).T

        result = mat.dot(vec)
        self.assertEqual(result.shape[0], 3)

        expected = mat_from_text("""
        0.0153882 0.0169049  0.017089
        """).T
        self.assertTrue(np.allclose(result, expected))

    def test_case2(self):
        mat = mat_from_text("""
  0.103762  0.0696809  -0.206449  -0.120681    0.20824  -0.111983   0.177561   0.626835  -0.510558   0.431474
  0.182125  -0.228064  0.0599508   0.582948   0.463558   0.236114   0.137867   0.407056  -0.209513   0.268628
  0.102353   0.613372   0.129175  -0.206235   0.311211 -0.0463518  -0.245578  -0.607952   0.106832  -0.117265
        """)
        vec = mat_from_text("""
  0.000413838  0.000406476  0.000492299   0.00241257  5.96886e-05  -0.00341331   0.00262761 -0.000787594   0.00641588   0.00165659
        """).T

        w = 0.5
        offset = mat_from_text("""
        0.340129 0.406907  0.07384
        """).T
        result = (mat.dot(vec) + offset) / w

        expected = mat_from_text("""
         0.675229  0.81338 0.148398
        """).T
        self.assertTrue(np.allclose(result, expected))

    def test_case3(self):
        np.random.seed(0)

        hasher = GaussianLSH(10, 5, 3, w=0.5)
        vectors = mat_from_text("""
   0.0042174   -0.0134243   0.00527203 -0.000760696  -0.00267559    -0.015297   -0.0311168   0.00506558   -0.0193664   0.00619034
  0.000413838  0.000406476  0.000492299   0.00241257  5.96886e-05  -0.00341331   0.00262761 -0.000787594   0.00641588   0.00165659
        """)

        hashes = list(hasher._iter_hashes(vectors[0]))
        self.assertEqual(len(hashes), 5)
