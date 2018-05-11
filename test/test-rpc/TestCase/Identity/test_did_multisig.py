from TestCase.MVSTestCase import *

class TestDIDMultiSig(MVSTestCaseBase):
    DID_ABC = "ABC.DID"
    DID_ACD = "ACD.DID"


    def setUp(self):
        MVSTestCaseBase.setUp(self)

        #create multisig addr
        self.ABC = Alice.new_multisigaddress("A&B&C", [Bob, Cindy], 2)
        self.ACD = Alice.new_multisigaddress("A&C&D", [Cindy, Dale], 2)


    def test_0_issuedid(self):
        #check or issue did to the corresponding multisig addr
        exist_symbols = []
        ec, message = mvs_rpc.list_dids()
        if ec == 0:
            exist_symbols = [i["symbol"] for i in message['dids']]

        for did, addr in [(self.DID_ABC, self.ABC), (self.DID_ACD, self.ACD)]:
            if did not in exist_symbols:
                Alice.send_etp(addr, 10 * (10 ** 8))
                Alice.mining()
                ec, message = mvs_rpc.issue_did(Alice.name, Alice.password, addr, did)
                import pdb;pdb.set_trace()
                self.assertEqual(ec, 0, message)

