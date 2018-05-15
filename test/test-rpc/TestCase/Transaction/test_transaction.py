import random
from TestCase.MVSTestCase import *

class TestTransaction(MVSTestCaseBase):

    def test_0_gettx(self):
        # the argument ('11111111111111111111') for option '--HASH' is invalid
        ec, message = mvs_rpc.gettx('1'*20)
        self.assertEqual(ec, 1021, message)

        # transaction does not exist!
        ec, message = mvs_rpc.gettx('18908d2035f40a9d4f1e07c1a3f16ace0882afbd31a7dfa3176843dd620abe9d')
        self.assertEqual(ec, 5306, message)

        # send etp to get a tx hash
        tx_hash = Alice.send_etp(Zac.mainaddress(), 10**8)
        ec, tx_json = mvs_rpc.gettx(tx_hash)
        self.assertEqual(ec, 0, tx_json)
        self.assertEqual(tx_json['height'], 0)

        Alice.mining()
        ec, tx_json2 = mvs_rpc.gettx(tx_hash)
        self.assertEqual(ec, 0, tx_json2)
        self.assertNotEqual(tx_json2['height'], 0)
        
        tx_json2['height'] = 0
        self.assertEqual(tx_json, tx_json2)


    def test_1_listtxs(self):
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)