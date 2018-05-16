# -*- coding: utf-8 -*-
import os
import unittest
import TestCase

def run_testcase():
    ret = []
    with open('mvs_test_report.txt', 'w') as f:
        category_lst = ['Account', "Blockchain", "Block", "Identity", "ETP", "Asset", "MultiSignature", "RawTx", "Transaction"]
        for i in category_lst:
            test_dir = "./TestCase/" + i
            print >> f, '====', i, '===='
            discover = unittest.defaultTestLoader.discover(test_dir, pattern='test_*.py', top_level_dir="./TestCase/")

            runner = unittest.TextTestRunner(stream=f, verbosity=2)
            runner.run(discover)


if __name__ == '__main__':
    run_testcase()
