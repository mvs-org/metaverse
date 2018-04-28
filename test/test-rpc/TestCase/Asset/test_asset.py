import unittest
import utils.mvs_rpc as mvs_rpc

from Roles import Alice, Zac

class TestAsset(unittest.TestCase):
    roles = [Alice, Zac]
    def setUp(self):
        for role in self.roles:
            result, message = role.create()
            self.assertEqual(result, 0, message)

    def tearDown(self):
        #import pdb;pdb.set_trace()
        for role in self.roles:
            result, message = role.delete()
            self.assertEqual(result, 0, message)

    def test_1_create_asset(self):
        Alice.create_asset(False)
        #validate result
        assets = Alice.get_accountasset()
        self.assertEqual(len(assets), 1)
        self.assertEqual(assets[0].symbol, Alice.asset_symbol)
        self.assertEqual(assets[0].address, "")
        self.assertEqual(assets[0].issuer, Alice.name)
        self.assertEqual(assets[0].status, 'unissued')

        #delete_localasset
        Alice.delete_localasset()
        assets = Alice.get_accountasset()
        self.assertEqual(len(assets), 0)

    def test_2_issue_asset_from(self):
        Alice.create_asset(False)
        Alice.issue_asset(Alice.mainaddress())
        Alice.mining()

        assets = Alice.get_accountasset()
        self.assertEqual(len(assets), 1)
        self.assertEqual(assets[0].symbol, Alice.asset_symbol)
        self.assertEqual(assets[0].address, Alice.mainaddress())
        self.assertEqual(assets[0].issuer, Alice.name)
        self.assertEqual(assets[0].status, 'unspent')

        addressassets = Alice.get_addressasset( Alice.mainaddress() )
        addressasset = filter(lambda a: a.symbol == Alice.asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 1)
        self.assertEqual(addressasset[0].symbol, Alice.asset_symbol)
        self.assertEqual(addressasset[0].address, Alice.mainaddress())
        self.assertEqual(addressasset[0].issuer, Alice.name)
        self.assertEqual(addressasset[0].status, 'unspent')








