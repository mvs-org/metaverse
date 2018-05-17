import random
import MOCs
from utils import mvs_rpc, validate, common
from TestCase.MVSTestCase import *

class TestDID(MVSTestCaseBase):
    need_mine = False

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

    def test_6_modify_did_boundary(self):
        # account password match error
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password + '1', Alice.addresslist[1], Alice.did_symbol)
        self.assertEqual(ec, 1000, message)

        # Did 'ZAC.DID' does not exist in blockchain
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password, Alice.addresslist[1], Zac.did_symbol)
        self.assertEqual(ec, 7006, message)

        # Did 'BOB.DID' is not owned by Alice
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password, Alice.addresslist[1], Bob.did_symbol)
        self.assertEqual(ec, 7009, message)

        # Target address is not owned by account.
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password, Bob.addresslist[1], Alice.did_symbol)
        self.assertEqual(ec, 4003, message)

        # Target address is already binded with some did in blockchain
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password, Alice.mainaddress(), Alice.did_symbol)
        self.assertEqual(ec, 7002, message)

    def test_7_modify_did(self):
        '''modify did between Zac's addresses'''
        temp_did = "ZAC.DID@" + common.get_timestamp()
        Alice.send_etp(Zac.mainaddress(), 10**8)
        Alice.mining()
        ec, message = mvs_rpc.issue_did(Zac.name, Zac.password, Zac.mainaddress(), temp_did)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        # no enough balance, unspent = 0, payment = 10000
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.addresslist[1], temp_did)
        self.assertEqual(ec, 3302,  message)

        Alice.send_etp(Zac.addresslist[1], 10 ** 4)
        Alice.mining()

        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.addresslist[1], temp_did)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        ec, message = mvs_rpc.list_dids(Zac.name, Zac.password)
        self.assertEqual(ec, 0, message)

        self.assertEqual(message['dids'][0]['symbol'], temp_did, message)
        self.assertEqual(message['dids'][0]['address'], Zac.addresslist[1], message)

        # confirm the modification procedure by list_didaddresses
        ec, message = mvs_rpc.list_didaddresses(Zac.name, Zac.password, temp_did)
        self.assertEqual(ec, 0, message)

        self.assertEqual(message['addresses'][0]["address"], Zac.addresslist[1])
        self.assertEqual(message['addresses'][0]["status"], "current")

        self.assertEqual(message['addresses'][1]["address"], Zac.addresslist[0])
        self.assertEqual(message['addresses'][1]["status"], "old")


    def test_8_list_didaddresses_boundary(self):
        ec, message = mvs_rpc.list_didaddresses(Alice.name, Alice.password+'1', Alice.did_symbol)
        self.assertEqual(ec, 1000, message)

        # did symbol does not exist in blockchain
        ec, message = mvs_rpc.list_didaddresses(Alice.name, Alice.password, Zac.did_symbol)
        self.assertEqual(ec, 7006, message)

    def test_9_modify_did_multisig(self):
        did_normal_symbal = "Zac@"+common.get_timestamp()
        Alice.send_etp(Zac.mainaddress(), 10 ** 8)
        Alice.mining()

        ec, message = mvs_rpc.issue_did(Zac.name, Zac.password, Zac.mainaddress(), did_normal_symbal)   
        self.assertEqual(ec, 0, message)
        Alice.mining()

        group = [Alice, Cindy, Dale,Frank, Zac]

        did_symbol = '@'.join(r.name for r in group) + common.get_timestamp()
        for i, role in enumerate(group):
            addr = role.new_multisigaddress("Alice & Cindy & Zac's Multisig-DID", group[:i] + group[i+1:], 3)
        
        Alice.send_etp(addr, (10 ** 9))      
        Alice.mining()

        ec, message = mvs_rpc.issue_did(group[0].name, group[0].password, addr, did_symbol)   
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, message['raw'])
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.sign_multisigtx(group[2].name, group[2].password, message['raw'], True)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        
        # did not find
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.mainaddress(), common.get_timestamp())
        self.assertEqual(ec, 7006,  message)

        #did not owner by account
        ec, message = mvs_rpc.modify_did(Bob.name, Bob.password, Bob.mainaddress(), did_symbol)
        self.assertEqual(ec, 7009,  message)

        #did address invalid
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, "Test"*20, did_symbol)
        self.assertEqual(ec, 4012,  message)

        #address didn't owned by the account
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Bob.mainaddress(), did_symbol)
        self.assertEqual(ec, 4003,  message)

        #address is already binded with did
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.mainaddress(), did_symbol)
        self.assertEqual(ec, 7002,  message)


        # no enough balance, unspent = 0, payment = 10000
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.addresslist[1], did_symbol)
        self.assertEqual(ec, 3302,  message)
        
        Alice.send_etp(Zac.addresslist[1], 10 ** 5)
        Alice.mining()

        #signure must be large than 3
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.addresslist[1], did_symbol)
        self.assertEqual(ec, 0, message)

        #cannot transfer to another multi-signature
        ec, message = mvs_rpc.sign_multisigtx(group[0].name, group[0].password, message['raw'], True)
        self.assertEqual(ec, 5304, message)
        
        group_new = [Bob, Dale, Zac]
        for i, role in enumerate(group_new):
            addr_new = role.new_multisigaddress(
                "Bob & Dale & Zac's Multisig-DID", group_new[:i] + group_new[i+1:], 2)

        Alice.send_etp(addr_new, (10 ** 6))
        Alice.mining()
        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, addr_new, did_symbol)
        self.assertEqual(ec, 7010, message)


        ec, message = mvs_rpc.modify_did(Zac.name, Zac.password, Zac.addresslist[1], did_symbol)
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.sign_multisigtx(group[0].name, group[0].password, message['raw'])
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, message['raw'], True)
        self.assertEqual(ec, 0, message)
        self.assertNotEqual(self.get_didaddress(Zac.name, Zac.password,did_symbol), Zac.addresslist[1], "Failed where modify did address from multi_signature to multi_signature address")

    def get_didaddress(self, account, password, symbol):
        ec, message = mvs_rpc.list_didaddresses(account, password, symbol)
        self.assertEqual(ec, 0, message)
        return message['addresses'][0]['address']



class TestDIDSendMore(MVSTestCaseBase):
    need_mine = False

    def test_0_didsend_more(self):
        receivers = {
            Bob.mainaddress(): 100000,
            Zac.did_symbol: 100001,
            Dale.mainaddress(): 100002,
            Eric.did_symbol: 100003,
        }
        specific_fee = 12421

        # password error
        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password + '1', receivers, Alice.addresslist[1], specific_fee)
        self.assertEqual(ec, 1000, message)

        # receivers contain not exits did -- Zac' did
        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password, receivers, Alice.addresslist[1], specific_fee)
        self.assertEqual(ec, 7006, message)

        #Zac -> Cindy
        receivers = {
            Bob.mainaddress(): 100000,
            Cindy.did_symbol: 100001,
            Dale.mainaddress(): 100002,
            Eric.did_symbol: 100003,
        }
        # mychange -- not exits did
        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password, receivers, Zac.did_symbol, specific_fee)
        self.assertEqual(ec, 7006, message)

    def test_1_didsend_more(self):
        receivers = {
            Bob.mainaddress(): 100000,
            Cindy.did_symbol: 100001,
            Dale.mainaddress(): 100002,
            Eric.did_symbol: 100003,
        }
        specific_fee = 12421


