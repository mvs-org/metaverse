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
        self.assertEqual(len(message), 0)

        # create 1 multi add
        addr = Alice.new_multisigaddress('A&B&C', [Bob, Cindy], 2)
        ec, message = mvs_rpc.list_multisig(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(len(message), 1, message)
        self.assertEqual(message[0]["address"], addr, message)

    def test_2_delete(self):
        '''test delete multisig addr'''
        # account password match error
        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password + '1', "x")
        self.assertEqual(ec, 1000, message)

        # invalid address
        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password, "x")
        self.assertEqual(ec, 4015, message)

        addr = Alice.new_multisigaddress('A&B&C', [Bob, Cindy], 2)
        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password, addr)
        self.assertEqual(ec, 0, message)

        #confirm by list
        ec, message = mvs_rpc.list_multisig(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(len(message), 0)

    def test_3_multi_sendasset(self):
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        total_amount = self.get_asset_amount(Alice, asset_symbol)
        self.assertGreater(total_amount, 0, "failed to create asset")

        group = [Alice, Bob, Cindy,Dale, Zac]
        address = common.create_multisig_address(group, 3)
        Alice.send_etp(address, 10**5)
        Alice.mining()

        # send amount asset to multi-signature address
        result, message = mvs_rpc.send_asset(Alice.name, Alice.password, address, asset_symbol, 2000)
        self.assertEqual(result, 0 , message)
        Alice.mining()

        #send from multisig-addr
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), 1500, 2 ,asset_symbol)
        self.assertEqual(result, 2003 , message)

        #invalid parameter
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), -1, 3 ,asset_symbol)
        self.assertEqual(result, 1021 , message)

        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), 200**10, 3 ,asset_symbol)
        self.assertEqual(result, 1021 , message)

        #no enough amount
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), total_amount+1000, 3 ,asset_symbol)
        self.assertEqual(result, 5001 , message)

        #invalid symbol
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), 100, 3 , "test"+common.get_timestamp())
        self.assertEqual(result, 5001 , message)

        #multisig of from address is not found
        result, message = mvs_rpc.create_multisigtx(Frank.name, Frank.password, address, Alice.mainaddress(), total_amount+1000, 3 ,asset_symbol)
        self.assertEqual(result, 5203 , message)

        #signature must be large than 3
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), 100, 3 , asset_symbol)
        self.assertEqual(result, 0 , message)

        result, message = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, message, True)
        self.assertEqual(result, 5304 , message)

        #success test case
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), 100, 3 , asset_symbol)
        self.assertEqual(result, 0 , message)

        result, message = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, message)
        self.assertEqual(result, 0 , message)

        tx = message["rawtx"]
        result, message = mvs_rpc.sign_multisigtx(group[2].name, group[2].password, tx, True)
        self.assertEqual(result, 0 , message)
        Alice.mining()
        self.assertEqual(self.get_asset_amount(group[0], asset_symbol, address), 2000-100,"Failed when send asset from multi-signature to normal")


    def get_asset_amount(self, role, asset_symbol, addr=None):
        if addr == None:
            addr = role.mainaddress()
        addressassets = role.get_addressasset(addr)

        #we only consider Alice's Asset
        addressasset = filter(lambda a: a.symbol == asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 1)
        previous_quantity = addressasset[0].quantity
        previous_decimal = addressasset[0].decimal_number
        return previous_quantity * (10 ** previous_decimal)