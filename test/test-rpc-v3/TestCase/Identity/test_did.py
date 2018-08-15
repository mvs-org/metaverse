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

class TestDIDSendMore(MVSTestCaseBase):
    def test_0_didsend_more(self):
        receivers = {
            Bob.mainaddress(): 100000,
            Cindy.did_symbol: 100001,
            Dale.mainaddress(): 100002,
            Eric.did_symbol: 100003,
        }
        specific_fee = 12421
        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password, receivers, Alice.addresslist[1], specific_fee)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        # change is did
        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password, receivers, Frank.did_symbol)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        # change is None
        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password, receivers)
        self.assertEqual(ec, 0, message)
        Alice.mining()

    def test_1_didsend_more(self):
        did_symbol = 'Zac@'+common.get_random_str()
        Alice.send_etp(Zac.mainaddress(), 10**8)
        Alice.mining()
        Zac.register_did(symbol=did_symbol)
        Alice.mining()

        receivers = {
            Zac.mainaddress(): 100000,
            did_symbol: 200000,
            Cindy.did_symbol: 100001,
            Dale.mainaddress(): 100002,
            Eric.did_symbol: 100003,
        }

        ec, message = mvs_rpc.didsendmore(Alice.name, Alice.password, receivers, Alice.did_symbol)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        self.assertEqual(300000,Zac.get_balance(),"sendmore failed")




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

    def get_asset_amount(self, role, asset_symbol):
        addressassets = role.get_addressasset(role.mainaddress())

        addressasset = filter(lambda a: a.symbol == asset_symbol, addressassets)
        if len(addressasset) == 1:
            previous_quantity = addressasset[0].quantity
            previous_decimal = addressasset[0].decimal_number
            return previous_quantity * (10 ** previous_decimal)
        elif len(addressasset) == 0:
            return 0
        self.assertEqual(0,1,addressasset)

    def test_2_didsend_asset(self):
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        # send to did
        pA = self.get_asset_amount(Alice, asset_symbol)
        pB = self.get_asset_amount(Bob, asset_symbol)
        tx_hash = Alice.didsend_asset(Bob.did_symbol, 1, asset_symbol)
        Alice.mining()
        cA = self.get_asset_amount(Alice, asset_symbol)
        cB = self.get_asset_amount(Bob, asset_symbol)

        self.assertEqual(pA, cA + 1)
        self.assertEqual(pB, cB - 1)

    def test_3_didsend_asset_from(self):
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        # send to did
        pA = self.get_asset_amount(Alice, asset_symbol)
        pB = self.get_asset_amount(Bob, asset_symbol)

        tx_hash = Alice.didsend_asset_from(Alice.did_symbol, Bob.did_symbol, 1, asset_symbol)
        Alice.mining()

        cA = self.get_asset_amount(Alice, asset_symbol)
        cB = self.get_asset_amount(Bob, asset_symbol)

        self.assertEqual(pA, cA + 1)
        self.assertEqual(pB, cB - 1)

class Testdidcommon(MVSTestCaseBase):
    def test_1_registerdid(self):
        special_symbol=['@','.','-','_']
        optional = {}
        for i in xrange(len(special_symbol)):
            optional[Zac.addresslist[i]] = 10**8

        mvs_rpc.sendmore(Alice.name, Alice.password, optional)
        Alice.mining()

        for i ,symbol in  enumerate(special_symbol):
            did_symbol = '%s%stest%d%s'%(Zac.did_symbol,symbol,i,common.get_random_str())
            ec, message = Zac.register_did(Zac.addresslist[i], did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()
            self.assertEqual(Zac.get_didaddress(did_symbol), Zac.addresslist[i], 'Failed when registerdid with:'+symbol)

    def test_2_didchangeaddress(self):
        did_symbol = 'Zac@'+common.get_random_str()
        Alice.send_etp(Zac.mainaddress(), 10**8)
        Alice.mining()

        ec, message = Zac.register_did(symbol=did_symbol)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        self.assertEqual(Zac.get_didaddress(did_symbol), Zac.mainaddress(), 'Failed when registerdid with:'+did_symbol)

        Alice.send_etp(Zac.addresslist[1], 10**4)
        Alice.mining()
        ec, message = mvs_rpc.change_did(Zac.name, Zac.password, Zac.addresslist[1], did_symbol)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        self.assertEqual(Zac.get_didaddress(did_symbol), Zac.addresslist[1], 'Failed when registerdid with:'+did_symbol)

        Alice.send_etp(Zac.mainaddress(), 10**4)
        Alice.mining()
        ec, message = mvs_rpc.change_did(Zac.name, Zac.password, Zac.mainaddress(), did_symbol)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        self.assertEqual(Zac.get_didaddress(did_symbol), Zac.mainaddress(), 'Failed when registerdid with:'+did_symbol)

class TestdidUTXOcommon(MVSTestCaseBase):
    def test_didsend_twice(self):
        Alice.send_etp(Zac.mainaddress(), 10**10)
        Alice.mining()

        ##registerdid
        did_symbol = 'Zac@'+common.get_random_str()
        ec, message = Zac.register_did(symbol=did_symbol)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #didsendfrom
        ec, message = mvs_rpc.didsend_from(Zac.name,Zac.password, did_symbol, Alice.mainaddress(),10000)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsend_from(Zac.name,Zac.password, did_symbol, Alice.mainaddress(),10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #didsend
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password,  Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #didsendmore
        receivers = {
            Bob.mainaddress(): 100000,
            Cindy.did_symbol: 100001,
            Dale.mainaddress(): 100002,
            Eric.did_symbol: 100003,
        }
        ec, message = mvs_rpc.didsendmore(Zac.name, Zac.password, receivers, did_symbol, 10000)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsendmore(Zac.name, Zac.password, receivers, did_symbol, 10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #create asset
        domain_symbol, asset_symbol = Zac.create_random_asset(did_symbol=did_symbol)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #sendasset
        ec, message = mvs_rpc.didsend_asset(Zac.name,Zac.password, Alice.did_symbol,asset_symbol, 100)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #sendassetfrom
        ec, message = mvs_rpc.didsend_asset_from(Zac.name,Zac.password, did_symbol,Alice.did_symbol,asset_symbol, 100)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #register mit
        mit_symbol = ("MIT." + common.get_random_str()).upper()
        content = "MIT of Zac: " + mit_symbol
        ec, message = mvs_rpc.register_mit(Zac.name, Zac.password, did_symbol, mit_symbol, content)
        self.assertEqual(ec, code.success, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        # transfer mit
        ec, message = Zac.transfer_mit(Bob.did_symbol, mit_symbol)
        self.assertEqual(ec, code.success, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()


        #issue cert
        cert_symbol = Zac.issue_naming_cert(domain_symbol,did_symbol)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #transfer cert
        ec, message = mvs_rpc.transfer_cert(Zac.name, Zac.password, Alice.did_symbol, cert_symbol,
                                            'naming',
                                            fee=None)
        self.assertEqual(ec, 0, message)
        ec, message = mvs_rpc.didsend(Zac.name,Zac.password, Alice.did_symbol,10000)
        self.assertEqual(ec, 0, message)
        Alice.mining()
