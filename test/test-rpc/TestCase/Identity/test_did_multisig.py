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

        if self.DID_ABC not in exist_symbols:
            ec, message = mvs_rpc.issue_did(Alice.name, Alice.password, self.ABC, self.DID_ABC)
            self.assertEqual(ec, 0, message)

        if self.DID_ACD not in exist_symbols:
            ec, message = mvs_rpc.issue_did(Alice.name, Alice.password, self.ACD, self.DID_ACD)
            self.assertEqual(ec, 0, message)

