from TestCase.MVSTestCase import *

class TestMultiSig(MVSTestCaseBase):
    need_mine = False
    def test_0_getnew(self):
        '''test getnew(_multisig'''
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

    def test_1_list(self):
        '''test list_multisig'''
        # account password match error
        ec, message = mvs_rpc.list_multisig(Alice.name, Alice.password + '1')
        self.assertEqual(ec, 1000, message)

        # no multisig addr
        ec, message = mvs_rpc.list_multisig(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(message["multisig"], None)

        # create 1 multi add
        addr = Alice.new_multisigaddress('A&B&C', [Bob, Cindy], 2)
        ec, message = mvs_rpc.list_multisig(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(len(message["multisig"]), 1, message)
        self.assertEqual(message["multisig"][0]["address"], addr, message)

    def test_2_delete(self):
        '''test delete multisig addr'''
        # account password match error
        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password + '1', "x")
        self.assertEqual(ec, 1000, message)

        # invalid address
        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password, "x")
        self.assertEqual(ec, 5203, message)

        addr = Alice.new_multisigaddress('A&B&C', [Bob, Cindy], 2)
        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password, addr)
        self.assertEqual(ec, 0, message)

        #confirm by list
        ec, message = mvs_rpc.list_multisig(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(message["multisig"], None)

