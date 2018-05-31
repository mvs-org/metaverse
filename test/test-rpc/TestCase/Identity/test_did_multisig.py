from utils import common
from TestCase.MVSTestCase import *

class TestDIDMultiSig(MVSTestCaseBase):

    def test_0_registerdid(self):
        group = [Alice, Bob, Zac]
        did_symbol = "Alice.Bob.Zac.DIID." + common.get_random_str()

        for i, role in enumerate(group):
            addr = role.new_multisigaddress("Alice & Bob & Zac's Multisig-DID", group[:i] + group[i+1:], 2)

        Alice.send_etp(addr, (10 ** 8))
        Alice.mining()

        ec, tx = mvs_rpc.register_did(group[0].name, group[0].password, addr, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx, True)
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

        did_symbol = '@'.join(r.name for r in group) + common.get_random_str()
        for i, role in enumerate(group):
            addr = role.new_multisigaddress("Alice & Cindy & Zac's Multisig-DID", group[:i] + group[i+1:], 2)

        Alice.send_etp(addr, (10 ** 9))
        Alice.mining()


        ec, tx = mvs_rpc.register_did(group[0].name, group[0].password, addr, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx, True)
        self.assertEqual(ec, 0, tx)
        Alice.mining()


        group_new = [ Bob, Dale, Zac]
        for i, role in enumerate(group_new):
            addr_new = role.new_multisigaddress(
                "Bob & Dale & Zac's Multisig-DID", group_new[:i] + group_new[i+1:], 2)

        Alice.send_etp(addr_new, (10 ** 6))
        Alice.mining()

        normal_new = Zac.mainaddress()
        Alice.send_etp(normal_new , 123456789)
        Alice.mining()

        ec, tx = mvs_rpc.change_did(Zac.name, Zac.password, normal_new, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, tx, True)
        self.assertEqual(ec, 0, tx)
        Alice.mining()

        did_address = group[0].get_didaddress(did_symbol)
        self.assertEqual(did_address, normal_new, "Failed where modify did address from multi_signature to normal ")

        normal_new = Zac.addresslist[1]
        Alice.send_etp(normal_new , (10 ** 6) )
        Alice.mining()
        ec, tx = mvs_rpc.change_did(Zac.name, Zac.password, normal_new, did_symbol)
        self.assertEqual(ec, 0, tx)
        Alice.mining()

        did_address = group[0].get_didaddress(did_symbol)
        self.assertEqual(did_address, normal_new, "Failed where modify did address from normal to normal")

        ec, tx = mvs_rpc.change_did(Zac.name, Zac.password, addr_new, did_symbol)
        self.assertEqual(ec, 0, tx)

        ec, tx = mvs_rpc.sign_multisigtx(group_new[0].name, group_new[0].password, tx, True)
        self.assertEqual(ec, 0, tx)
        Alice.mining()

        did_address = group[0].get_didaddress(did_symbol)
        self.assertEqual(did_address, addr_new, "Failed where modify did address from normal to multi_signature address")


class TestWithExistDID(MultiSigDIDTestCase):
    def test_0_todo(self):
        pass


