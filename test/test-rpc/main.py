# -*- coding: utf-8 -*-
import os
import unittest
import TestCase
from Roles import *
from utils import mvs_rpc
from HTMLTestRunner import HTMLTestRunner

def clear_account():
    for role in [Alice, Bob, Cindy, Dale, Eric, Frank, Zac]:
        try:
            # check if the role exists by get_balance
            role.get_balance()
            role.delete()
        except:
            pass

def ensure_Alice_balance():
    Alice.create()
    try:
        while Alice.get_balance() < 1000 * (10**8):
            Alice.mining(100)
    finally:
        Alice.delete()

def run_testcase():
    with open('mvs_test_report.html', 'w') as f:
        # runner = unittest.TextTestRunner(stream=f, verbosity=2)
        runner = HTMLTestRunner(stream=f,
                                title='MVS API Test Report',
                                description='',
                                verbosity=2)

        #category_lst = ['Account', "Blockchain", "Block", "Identity", "ETP", "Asset", "MultiSignature", "RawTx", "Transaction"]
        #for i in category_lst:
        test_dir = "./TestCase/"
        discover = unittest.defaultTestLoader.discover(test_dir, pattern='test_*.py', top_level_dir="./TestCase/")
        runner.run(discover)
    for i in mvs_rpc.RPC.export_method_time():
        print i


if __name__ == '__main__':
    clear_account()
    ensure_Alice_balance()
    run_testcase()
