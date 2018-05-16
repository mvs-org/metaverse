from utils import common
from TestCase.MVSTestCase import *

class TestDIDMultiSig(MVSTestCaseBase):
    DID_ABC = "ABC.DID"
    DID_DEF = "DEF.DID"

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
        '''modify did to multisig-address'''
        self.assertEqual(0,1,"function not realized yet.")