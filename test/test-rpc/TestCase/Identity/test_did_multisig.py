from utils import common
from TestCase.MVSTestCase import *

class TestDIDMultiSig(MVSTestCaseBase):

    def test_0_issuedid(self):
        group = [Alice, Bob, Zac]
        did_symbol = "Alice.Bob.Zac.DID." + common.get_timestamp()

        for i, role in enumerate(group):
            addr = role.new_multisigaddress("Alice & Bob & Zac's Multisig-DID", group[:i] + group[i+1:], 2)

        Alice.send_etp(addr, (10 ** 8))
        Alice.mining()

        ec, tx = mvs_rpc.issue_did(group[0].name, group[0].password, addr, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx['raw'], True)
        self.assertEqual(ec, 0, tx)

        Alice.mining()

        ec, message = mvs_rpc.list_dids()
        self.assertEqual(ec, 0, message)

        for did_ in message['dids']:
            if did_['symbol'] == did_symbol and did_["address"] == addr:
                break
        else:
            self.assertEqual(0, 1, "did -> multi-sig addr not found error")


    def test_1_modifydid(self):
        #import pdb; pdb.set_trace()
        group = [Alice, Cindy, Zac]

        did_symbol = '@'.join(r.name for r in group) + common.get_timestamp()
        for i, role in enumerate(group):
            addr = role.new_multisigaddress("Alice & Cindy & Zac's Multisig-DID", group[:i] + group[i+1:], 2)
        
        Alice.send_etp(addr, (10 ** 9))      
        Alice.mining()


        ec, tx = mvs_rpc.issue_did(group[0].name, group[0].password, addr, did_symbol)   
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx['raw'], True)
        self.assertEqual(ec, 0, tx)
        Alice.mining()


        group_new = [Alice, Bob, Zac]
        for i, role in enumerate(group_new):
            addr_new = role.new_multisigaddress(
                "Alice & Bob & Zac's Multisig-DID", group_new[:i] + group_new[i+1:], 2)

        Alice.send_etp(addr_new, (10 ** 6))
        Alice.mining()

        print "from multi_signature address to normal address"
        print "current address:"+self.get_didaddress(group[0].name, group[0].password, did_symbol)
        
        normal_new = Zac.mainaddress()
        Alice.send_etp(normal_new , 123456789)
        Alice.mining()
    
        ec, tx = mvs_rpc.modify_did(Zac.name, Zac.password, normal_new, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx['raw'], True)
        self.assertEqual(ec, 0, tx)
        Alice.mining()
        print "after modify address:"+self.get_didaddress(Zac.name,Zac.password, did_symbol)

        print "from normal address  address to normal address"      
        normal_new = Zac.addresslist[1]
        Alice.send_etp(normal_new , (10 ** 6) )
        Alice.mining()
        ec, tx = mvs_rpc.modify_did(Zac.name, Zac.password, normal_new, did_symbol)
        self.assertEqual(ec, 0, tx)
        Alice.mining()
        print "after modify address:"+self.get_didaddress(Zac.name,Zac.password, did_symbol)

        print "from normal address to multi_signature address"

        ec, tx = mvs_rpc.modify_did(Zac.name, Zac.password, addr, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[0].name, group[0].password, tx['raw'])
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx['raw'], True)
        self.assertEqual(ec, 0, tx)
        Alice.mining(10)
        print "after modify address:"+self.get_didaddress(Zac.name,Zac.password, did_symbol)

        print "from multi_signature address to multi_signature"
        ec, tx = mvs_rpc.modify_did(group[0].name, group[0].password, addr_new, did_symbol)
        self.assertEqual(ec, 70010, tx)


    def get_didaddress(self, account, password, symbol):
        ec, message = mvs_rpc.list_didaddresses(account, password, symbol)
        self.assertEqual(ec, 0, message)
        return message['addresses'][0]['address']





