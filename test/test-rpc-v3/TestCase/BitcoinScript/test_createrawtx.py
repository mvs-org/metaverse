from TestCase.MVSTestCase import *


class TestCreateRawTX(MVSTestCaseBase):
    need_mine = False
    def test_0_createrawtx(self):
        create_rawtx = lambda utxos: mvs_rpc.create_rawtx(0, [], {Zac.mainaddress():10**8}, mychange=Alice.mainaddress(), utxos=utxos)

        utxos = [("32" * 32, 0, 0xFFFFFFFF)]
        ec, message = create_rawtx(utxos)
        self.assertNotEqual(ec, 0, message)

        #send to get a valid tx hash
        txhash = Alice.send_etp(Zac.addresslist[1], 2* 10**8)

        ec, message = create_rawtx([(txhash, 10, 0xFFFFFFFF)])
        self.assertNotEqual(ec, 0, message)

        ec, message = create_rawtx([(txhash, 0, 0xFFFFFFFF)])
        self.assertEqual(ec, 0, message)
    



