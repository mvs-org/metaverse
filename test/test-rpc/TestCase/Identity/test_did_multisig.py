from TestCase.MVSTestCase import *

class TestDIDMultiSig(MVSTestCaseBase):
    DID_ABC = "ABC.DID"
    DID_DEF = "DEF.DID"

    def test_0_issuedid(self):
        # create multisig addr
        multisig_addrs = {}

        group1, group2 = [Alice, Bob, Cindy], [Dale, Eric, Frank]
        addr1_desc = "&".join([i.name for i in group1])
        addr2_desc = "&".join([i.name for i in group2])

        multisig_groups = {
            addr1_desc : group1,
            addr2_desc : group2
        }

        for addr_desc in multisig_groups:
            group = multisig_groups[addr_desc]
            for i, role in enumerate(group):
                addr = multisig_addrs[addr_desc] = role.new_multisigaddress(addr_desc, group[:i] + group[i+1:], 2)
                multisig_addrs[addr_desc] = addr

        #check or issue did to the corresponding multisig addr
        exist_symbols = []
        ec, message = mvs_rpc.list_dids()
        if ec == 0:
            exist_symbols = [i["symbol"] for i in message['dids']]

        for did, addr_desc in [ (self.DID_ABC, addr1_desc ),  (self.DID_DEF, addr2_desc)]:
            if did not in exist_symbols:
                addr = multisig_addrs[addr_desc]

                Alice.send_etp(addr, (10 ** 8))
                Alice.mining()
                roles = multisig_groups[addr_desc]

                ec, tx = mvs_rpc.issue_did(roles[0].name, roles[0].password, addr, did)
                self.assertEqual(ec, 0, tx)

                ec, tx = mvs_rpc.sign_multisigtx(roles[1].name, roles[1].password, tx, True)
                self.assertEqual(ec, 0, tx)

                Alice.mining()

                ec, message = mvs_rpc.list_dids()
                self.assertEqual(ec, 0, message)

                for did_ in message['dids']:
                    if did_['symbol'] == did and did_["address"] == addr:
                        break
                else:
                    self.assertEqual(0, 1, "did -> multi-sig addr not found error")

    def test_1_modifydid(self):
        '''modify did to multisig-address'''
        self.assertEqual(0,1,"function not realized yet.")