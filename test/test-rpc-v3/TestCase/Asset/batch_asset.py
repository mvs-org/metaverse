from utils import common, mvs_rpc
from TestCase.MVSTestCase import *

class TestAssetBatch(MVSTestCaseBase):
    need_mine = False
    def test_0_getaccountasset(self):
        '''create 3000 address multisig address for Alice, and then ...'''
        address_amount = 3000
        addresses = mvs_rpc.new_address(Alice.name, Alice.password, address_amount)
        round = 5000

        max_duration = 0.01
        avg_duration = 0.002
        durations = []

        for i in range(round):
            print i
            duration, ret = common.duration_call(mvs_rpc.get_accountasset, Alice.name, Alice.password)
            self.assertEqual(ret[0], 0, "mvs_rpc.get_accountasset failed!")
            self.assertLess(duration, max_duration)
            durations.append(duration)
        self.assertLess(sum(durations), avg_duration * round)

    def test_1_getaddressasset(self):
        round = 1000

        max_duration = 0.01
        avg_duration = 0.005
        durations = []

        for i in range(round):
            duration, ret = common.duration_call(mvs_rpc.get_addressasset, Alice.mainaddress())
            self.assertEqual(ret[0], 0, "mvs_rpc.get_addressasset failed!")
            self.assertLess(duration, max_duration)
            durations.append(duration)
        self.assertLess(sum(durations), avg_duration * round)