from TestCase.MVSTestCase import *

class TestMultiSigasset(MVSTestCaseBase):
    def test_0_multisig_asset(self):
        #create multisig addr
        Alice.create_asset()
        Alice.mining()
        amount_total = self.get_asset_amount(Alice)
        amount = 2000
        amount_ret = 1000

        group = [Alice, Bob, Zac]
        address = common.create_multisig_address(group, 2)
        Alice.send_etp(address, 10**4)
        Alice.mining()

        pre_asset = self.get_asset_amount(Alice)
        # send amount asset to multi-signature address
        result, message = mvs_rpc.send_asset(Alice.name, Alice.password, address, Alice.asset_symbol, amount)
        self.assertEqual(result, 0 , message)
        Alice.mining()
        self.assertEqual(amount, self.get_asset_amount(group[0],address) , "Failed when send asset from Alice to multi-signature address")

        #send from multisig-addr
        result, message = mvs_rpc.create_multisigtx(group[0].name, group[0].password, address, Alice.mainaddress(), amount_ret, 3 ,Alice.asset_symbol)
        self.assertEqual(result, 0 , message)

        ec, message = mvs_rpc.sign_multisigtx(group[1].name, group[1].password, message, True)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        self.assertEqual(amount_total-amount+amount_ret, self.get_asset_amount(Alice), "Failed  when send asset from multi-signature address to Alice")



    def get_asset_amount(self, role, addr=None):
        if addr == None:
            addr = role.mainaddress()
        addressassets = role.get_addressasset(addr)

        #we only consider Alice's Asset
        addressasset = filter(lambda a: a.symbol == Alice.asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 1)
        previous_quantity = addressasset[0].quantity
        previous_decimal = addressasset[0].decimal_number
        return previous_quantity * (10 ** previous_decimal)