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

    def test_2_listtxs(self):
        # account not found or incorrect password
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password+'1')
        self.assertEqual(ec, 1000, message)

        # invalid address parameter!
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, Alice.mainaddress()+'1')
        self.assertEqual(ec, 4010, message)

        ec, message = mvs_rpc.get_blockheader()
        self.assertEqual(ec, 0, message)

        height = message["number"]

        # height: firt < second
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, Alice.mainaddress(), [height-1, height])
        self.assertEqual(ec, 0, message)

        # height: firt == second
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, Alice.mainaddress(), [height, height])
        self.assertEqual(ec, 5103, message)

        # height: firt > second
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, Alice.mainaddress(), [height, height-1])
        self.assertEqual(ec, 5103, message)

        # index: page index parameter must not be zero
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, index=0)
        self.assertEqual(ec, 2003, message)

        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, index=1)
        self.assertEqual(ec, 0, message)

        # limit: page record limit parameter must not be zero
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, limit=0)
        self.assertEqual(ec, 2003, message)

        # limit: page record limit must not be bigger than 100.
        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, limit=101)
        self.assertEqual(ec, 2003, message)

        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, limit=100)
        self.assertEqual(ec, 0, message)


