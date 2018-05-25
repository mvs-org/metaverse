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
        result = runner.run(discover)

        #add api call time statics
        benchmark_head = '''
<table id='benchmark_table'>
<colgroup>
<col align='left' />
<col align='right' />
<col align='right' />
<col align='right' />
<col align='right' />
<col align='right' />
</colgroup>
<tr id='header_row'>
    <td>MVS API/Method</td>
    <td>Max(ms)</td>
    <td>Min(ms)</td>
    <td>Average(ms)</td>
    <td>Called(times)</td>
</tr>'''
        benchmark_fmt = '''
<tr class='passClass'>
    <td>%s</td>
    <td>%s</td>
    <td>%s</td>
    <td>%s</td>
    <td>%s</td>
</tr>'''
        benchmark_tail = '''
</table>'''
        body = ''.join( [benchmark_fmt % i for i in mvs_rpc.RPC.export_method_time()] )
        runner.ENDING_TMPL += benchmark_head + body + benchmark_tail
        f.seek(0)
        runner.generateReport(None, result)



if __name__ == '__main__':
    clear_account()
    ensure_Alice_balance()
    run_testcase()
