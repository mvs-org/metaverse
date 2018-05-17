# -*- coding: utf-8 -*-
import os
import unittest
import TestCase
from Roles import *

def clear_account():
    if os.path.exists('./Zac.txt'):
        with open('./Zac.txt') as f:
            lastword = f.read()
            Zac.mnemonic=[lastword.strip()]
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
    with open('mvs_test_report.txt', 'w') as f:
        category_lst = ['Account', "Blockchain", "Block", "Identity", "ETP", "Asset", "MultiSignature", "RawTx", "Transaction"]
        for i in category_lst:
            test_dir = "./TestCase/" + i
            print >> f, '====', i, '===='
            discover = unittest.defaultTestLoader.discover(test_dir, pattern='test_*.py', top_level_dir="./TestCase/")

            runner = unittest.TextTestRunner(stream=f, verbosity=2)
            runner.run(discover)


if __name__ == '__main__':
    clear_account()
    ensure_Alice_balance()
    run_testcase()
