from TestCase.MVSTestCase import *

class TestTxLocktime(MVSTestCaseBase):
    def test_0_locktime(self):
        ec, (height, _) = mvs_rpc.get_info()
        self.assertEqual(ec, 0)

        ec, messge = mvs_rpc.send(Alice.name, Alice.password, Zac.mainaddress(), 10**8, locktime=1 + height)
        self.assertNotEqual(ec, 0, messge)
        ec, messge = mvs_rpc.send(Alice.name, Alice.password, Zac.mainaddress(), 10 ** 8, locktime=0 + height)
        self.assertEqual(ec, 0, messge)
        ec, messge = mvs_rpc.send(Alice.name, Alice.password, Zac.mainaddress(), 10 ** 8, locktime=-1 + height)
        self.assertEqual(ec, 0, messge)
