import random
import MOCs
from utils import mvs_rpc, validate, common
from TestCase.MVSTestCase import *

class TestDID(MVSTestCaseBase):
    def test_0_boundary(self):
        # account password match error
        ec, message = mvs_rpc.issue_did(Zac.name, Zac.password+'1', Zac.mainaddress(), Zac.did_symbol)
        self.assertEqual(ec, 1000, message)

        #not enough fee
        ec, message = mvs_rpc.issue_did(Zac.name, Zac.password, Zac.mainaddress(), Zac.did_symbol, 10 ** 8 -1)
        self.assertEqual(ec, 7005, message)

        #not enough balance
        ec, message = mvs_rpc.issue_did(Zac.name, Zac.password, Zac.mainaddress(), Zac.did_symbol, 10 ** 8)
        self.assertEqual(ec, 3302, message)

        #did symbol duplicated
        ec, message = mvs_rpc.list_dids()
        if ec != 0:
            return
        exist_symbols = [i["symbol"] for i in message['dids']]
        for symbol in exist_symbols:
            ec, message = mvs_rpc.issue_did(Zac.name, Zac.password, Zac.mainaddress(), symbol, 10 ** 8)
            self.assertEqual(ec, 7002, message)

    def test_1_issue_did(self):
        '''
        this test case will create did for all roles. If not created before.
        '''
        for role in self.roles[:-1]:
            ec, message = mvs_rpc.list_dids(role.name, role.password)
            if ec == 0 and message['dids']:
                pass
            else:
                if role != Alice:
                    Alice.send_etp(role.mainaddress(), 10**8)
                    Alice.mining()
                ec, message = role.issue_did()
                self.assertEqual(ec, 0, message)
                Alice.mining()
                ec, message = mvs_rpc.list_dids(role.name, role.password)
            self.assertIn(role.did_symbol, [i["symbol"] for i in message['dids']], message)
        #address in use
        random_did_symbol = common.get_timestamp()+str(random.randint(1, 100))
        ec, message = mvs_rpc.issue_did(Alice.name, Alice.password, Alice.mainaddress(), random_did_symbol)
        self.assertEqual(ec, 7002, message)


    def test_2_didsend_etp(self):
        # account password match error
        ec, message = mvs_rpc.didsend(Alice.name, Alice.password + '1', Zac.mainaddress(), 10**5, 10**4, "test_2_didsend_etp")
        self.assertEqual(ec, 1000, message)

        # not enough balance
        ec, message = mvs_rpc.didsend(Zac.name, Zac.password, Alice.did_symbol, 10 ** 5, 10 ** 4, "test_2_didsend_etp")
        self.assertEqual(ec, 5302, message)

    def test_3_didsend_etp_from(self):
        # account password match error
        ec, message = mvs_rpc.didsend_from(Alice.name, Alice.password + '1', Alice.mainaddress(), Zac.mainaddress(), 10 ** 5, 10 ** 4, "test_3_didsend_etp_from")
        self.assertEqual(ec, 1000, message)

        # not enough balance
        ec, message = mvs_rpc.didsend_from(Zac.name, Zac.password, Zac.mainaddress(), Alice.did_symbol, 10 ** 5, 10 ** 4, "test_3_didsend_etp_from")
        self.assertEqual(ec, 3302, message)

    def test_4_didsend_asset(self):
        # account password match error
        ec, message = mvs_rpc.didsend_asset(Alice.name, Alice.password + '1', Zac.mainaddress(), Alice.asset_symbol, 10 ** 5, 10 ** 4)
        self.assertEqual(ec, 1000, message)

    def test_5_didsend_asset_from(self):
        # account password match error
        ec, message = mvs_rpc.didsend_asset_from(Alice.name, Alice.password + '1', Alice.mainaddress(), Zac.mainaddress(), Alice.asset_symbol, 10 ** 5, 10 ** 4)
        self.assertEqual(ec, 1000, message)
