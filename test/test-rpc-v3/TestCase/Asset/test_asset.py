from TestCase.MVSTestCase import *

class TestAsset(MVSTestCaseBase):

    def test_1_create_asset(self):
        Alice.ensure_balance()

        domain_symbol, asset_symbol = Alice.create_random_asset(is_issue=False)
        Alice.mining()

        #validate result
        assets = Alice.get_accountasset(asset_symbol)
        self.assertEqual(len(assets), 1)
        self.assertEqual(assets[0].symbol, asset_symbol)
        self.assertEqual(assets[0].address, "")
        self.assertEqual(assets[0].issuer, Alice.did_symbol)
        self.assertEqual(assets[0].status, 'unissued')

        #delete_localasset
        Alice.delete_localasset(asset_symbol)
        assets = Alice.get_accountasset(asset_symbol)
        self.assertEqual(len(assets), 0)

    def test_2_issue_asset(self):
        Alice.create_asset()
        Alice.mining()

        account_assets = Alice.get_accountasset()
        found_assets = list( filter(lambda a: a.symbol == Alice.asset_symbol, account_assets) )
        self.assertEqual(len(found_assets), 1)
        self.assertEqual(found_assets[0].symbol, Alice.asset_symbol)
        self.assertEqual(found_assets[0].issuer, Alice.did_symbol)
        self.assertEqual(found_assets[0].address, Alice.didaddress())
        self.assertEqual(found_assets[0].status, 'unspent')

        origin_amount = self.get_asset_amount(Alice)
        self.assertGreater(origin_amount, 0)

    def get_asset_amount(self, role, address=None):
        if address == None:
            address = role.didaddress()
        address_assets = role.get_addressasset(address)

        #we only consider Alice's Asset
        found_assets = list( filter(lambda a: a.symbol == Alice.asset_symbol, address_assets) )
        self.assertEqual(len(found_assets), 1)

        previous_quantity = found_assets[0].quantity
        previous_decimal = found_assets[0].decimal_number
        return previous_quantity * (10 ** previous_decimal)

    def test_3_sendasset(self):
        Alice.create_asset()
        Alice.mining()

        origin_amount = self.get_asset_amount(Alice)
        send_amount = 100
        #pre-set condition
        self.assertGreater(origin_amount, send_amount)
        Alice.send_asset(Zac.mainaddress(), send_amount)
        Alice.mining()

        final_amount = self.get_asset_amount(Alice)
        self.assertEqual(origin_amount - send_amount, final_amount)
        self.assertEqual(send_amount, self.get_asset_amount(Zac, Zac.mainaddress()))

    def test_4_sendassetfrom(self):
        Alice.create_asset()
        Alice.mining()

        origin_amount = self.get_asset_amount(Alice)
        send_amount = 100

        # pre-set condition
        self.assertGreater(origin_amount, send_amount)

        Alice.send_etp(Alice.didaddress(), 1 * 10 ** 8)
        Alice.mining()

        Alice.send_asset_from(Alice.didaddress(), Zac.mainaddress(), send_amount)
        Alice.mining()

        final_amount = self.get_asset_amount(Alice)
        self.assertEqual(origin_amount - send_amount, final_amount)
        self.assertEqual(send_amount, self.get_asset_amount(Zac, Zac.mainaddress()))

    def test_5_burn_asset(self):
        Alice.create_asset()
        Alice.mining()

        #use the asset created in the previous test case
        #amout > previous_amount
        amount = self.get_asset_amount(Alice)
        ec, message = mvs_rpc.burn(Alice.name, Alice.password, Alice.asset_symbol, amount + 1)
        self.assertEqual(ec, 5001, message)

        Alice.burn_asset(amount-1)
        Alice.mining()

        current_amount = self.get_asset_amount(Alice)
        self.assertEqual(current_amount, 1)

        Alice.burn_asset(1)
        Alice.mining()
        addressassets = Alice.get_addressasset(Alice.didaddress())
        addressasset = list( filter(lambda a: a.symbol == Alice.asset_symbol, addressassets) )
        self.assertEqual(len(addressasset), 0)

    def test_6_sendmoreasset(self):
        Alice.create_asset()
        Alice.mining()

        origin_amount = self.get_asset_amount(Alice)

        mvs_rpc.new_address(Zac.name, Zac.password, 2)
        receivers={
            Zac.addresslist[0]:1000,
            Zac.addresslist[1]:2000
        }

        #pre-set condition
        self.assertGreater(origin_amount, 3000)
        Alice.sendmore_asset(receivers, Alice.asset_symbol)
        Alice.mining()

        [ self.assertEqual(self.get_asset_amount(Zac, k), v) for k,v in receivers.items() ]
