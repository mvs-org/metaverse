from TestCase.MVSTestCase import *

class TestMultiSig(MVSTestCaseBase):
    def test_getnew(self):
        # account password match error
        ec, message = mvs_rpc.getnew_multisig(Alice.name, Alice.password + '1', "test", Alice.mainaddress(), [Bob.mainaddress(), Cindy.mainaddress()], 2)
        self.assertEqual(ec, 1000, message)

        #public key empty
        ec, message = mvs_rpc.getnew_multisig(Alice.name, Alice.password, "test", Alice.mainaddress(), [], 2)
        self.assertEqual(ec, 5201, message)

        # m = 0
        ec, message = mvs_rpc.getnew_multisig(Alice.name, Alice.password, "test", Alice.mainaddress(), [Bob.mainaddress(), Cindy.mainaddress()], 0)
        self.assertEqual(ec, 5220, message)

        # n > 20
        #ec, message = mvs_rpc.getnew_multisig(Alice.name, Alice.password, "test", Alice.get_publickey(Alice.mainaddress()), [Bob.get_publickey(Bob.mainaddress())]*20, 2)
        #self.assertEqual(ec, 1000, message)

        #not belongs to this account
        ec, message = mvs_rpc.getnew_multisig(Alice.name, Alice.password, "test",
                                              Alice.mainaddress(),
                                              [Bob.get_publickey(Bob.mainaddress()),
                                               Cindy.get_publickey(Cindy.mainaddress())], 2)
        self.assertEqual(ec, 5231, message)

        #multisig already exists.
        Alice.new_multisigaddress("test_getnew", [Bob, Cindy], 2)

        ec, message = mvs_rpc.getnew_multisig(Alice.name, Alice.password, "test", Alice.get_publickey( Alice.mainaddress() ),
                                              [Bob.get_publickey( Bob.mainaddress() ), Cindy.get_publickey( Cindy.mainaddress() )], 2)
        self.assertEqual(ec, 5202, message)