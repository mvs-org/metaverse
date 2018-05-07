import MOCs
from utils import validate

from TestCase.MVSTestCase import *

class TestDIDSend(MVSTestCaseBase):
    @classmethod
    def setUpClass(cls):
        #check if the did are created.
        ec, message = mvs_rpc.list_dids()
        if ec != 0:
            return
        exist_symbols = [i["symbol"] for i in message['dids']]

        assert (Alice.did_symbol in exist_symbols)
        assert (Bob.did_symbol in exist_symbols)

    def test_0_didsend_etp(self):
        #send to did
        tx_hash = Alice.didsend_etp(Bob.did_symbol, 12345)
        Alice.mining()
        validate.validate_tx(self, tx_hash, Alice, Bob, 12345, 10**4)

        #send to address
        tx_hash = Alice.didsend_etp(Zac.mainaddress(), 54321)
        Alice.mining()
        validate.validate_tx(self, tx_hash, Alice, Zac, 54321, 10 ** 4)

    def test_1_didsend_etp_from(self):
        # did -> did
        tx_hash = Alice.didsend_etp_from(Alice.did_symbol, Bob.did_symbol, 12345)
        Alice.mining()
        validate.validate_tx(self, tx_hash, Alice, Bob, 12345, 10 ** 4)

        # did -> addr
        tx_hash = Alice.didsend_etp_from(Alice.did_symbol, Zac.mainaddress(), 54321)
        Alice.mining()
        validate.validate_tx(self, tx_hash, Alice, Zac, 54321, 10 ** 4)

        # addr -> did
        tx_hash = Alice.didsend_etp_from(Alice.mainaddress(), Bob.did_symbol, 56789)
        Alice.mining()
        validate.validate_tx(self, tx_hash, Alice, Bob, 56789, 10 ** 4)

        # addr -> addr
        tx_hash = Alice.didsend_etp_from(Alice.mainaddress(), Bob.mainaddress(), 98765)
        Alice.mining()
        validate.validate_tx(self, tx_hash, Alice, Bob, 98765, 10 ** 4)

class TestDIDSendAsset(MVSTestCaseBase):
    @classmethod
    def setUpClass(cls):
        # check if the did are created.
        ec, message = mvs_rpc.list_dids()
        if ec != 0:
            return
        exist_symbols = [i["symbol"] for i in message['dids']]

        assert (Alice.did_symbol in exist_symbols)
        assert (Bob.did_symbol in exist_symbols)

    def get_asset_amount(self, role):
        addressassets = role.get_addressasset(role.mainaddress())

        #we only consider Alice's Asset
        addressasset = filter(lambda a: a.symbol == Alice.asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 1)
        previous_quantity = addressasset[0].quantity
        previous_decimal = addressasset[0].decimal_number
        return previous_quantity * (10 ** previous_decimal)

    def test_2_didsend_asset(self):
        Alice.create_asset()
        Alice.mining()
        # send to did
        import pdb; pdb.set_trace()
        pA = self.get_asset_amount(Alice)
        pB = self.get_asset_amount(Bob)
        tx_hash = Alice.didsend_asset(Bob.did_symbol, 1, Alice.asset_symbol)
        Alice.mining()
        cA = self.get_asset_amount(Alice)
        cB = self.get_asset_amount(Bob)

        self.assertEqual(pA, cA + 1)
        self.assertEqual(pB, cB - 1)

    def test_3_didsend_asset_from(self):
        Alice.create_asset()
        Alice.mining()
        # send to did
        pA = self.get_asset_amount(Alice)
        pB = self.get_asset_amount(Bob)

        tx_hash = Alice.didsend_asset_from(Alice.did_symbol, Bob.did_symbol, 1, Alice.asset_symbol)
        Alice.mining()
        cA = self.get_asset_amount(Alice)
        cB = self.get_asset_amount(Bob)

        self.assertEqual(pA, cA + 1)
        self.assertEqual(pB, cB - 1)

