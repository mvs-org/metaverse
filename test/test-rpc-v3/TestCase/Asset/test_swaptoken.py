import copy, time
import MOCs
from utils import common
from TestCase.MVSTestCase import *

# This asset is expected to be enough to be spend for 1 month
AssetName = 'ERC.TSTCASE.' + time.strftime("%04Y%02m",  time.localtime())


class TestSwapToken(MVSTestCaseBase):
    need_mine = False
    @classmethod
    def setUpClass(cls):
        MVSTestCaseBase.setUpClass()
        global AssetName
        ec, message = mvs_rpc.list_assets()
        assert (ec == 0)
        #message = message['assets']
        for i in message:
            if AssetName == i["symbol"]:
                break
        else:
            ec, message = Alice.create_asset_with_symbol(AssetName)
            assert (ec == 0)
            Alice.mining()
    @classmethod
    def tearDownClass(cls):
        Alice.mining()
        MVSTestCaseBase.tearDownClass()

    def test_0_check_positional_args(self):
        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, Alice.asset_symbol, 100, "{}")
        self.assertEqual(message, 'value of [type] not expect in message.')

        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, Alice.asset_symbol, 100, '{"type":"ETHX"}')
        self.assertEqual(message, 'value of [type] not expect in message.')

        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, Alice.asset_symbol, 100, '{"type":"ETH", "address":"xxx"}')
        self.assertEqual(message, 'value of [address] not expect in message.')

        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, Alice.asset_symbol, 100,
                                         '{"type":"ETH", "address":"0x000000000000000000000000000000000000000g"}')
        self.assertEqual(message, 'value of [address] not expect in message.')
        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, Alice.asset_symbol, 100,
                                         '{"type":"ETH", "address":"0x000000000000000000000000000000000000000"}')
        self.assertEqual(message, 'value of [address] not expect in message.')

        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, Alice.asset_symbol, 100,
                                         '{"type":"ETH", "address":"0x0000000000000000000000000000000000000000"}')
        self.assertEqual(message, "Only support assets prefixed by 'ERC.'")

        ec, message = mvs_rpc.swap_token(Alice.name, Alice.password, "ERC.ABC", 100,
                                         '{"type":"ETH", "address":"0x0000000000000000000000000000000000000000"}')
        self.assertEqual(message, 'not enough asset amount, unspent = 0, payment = 100')

    def test_1_check_optional_args(self):
        func = lambda change, from_, fee, swapfee:  mvs_rpc.swap_token(Alice.name, Alice.password, "ERC.ABC", 100,
                                         '{"type":"ETH", "address":"0x0000000000000000000000000000000000000000"}',
                                          change, from_, fee, swapfee)
        ec, message = func(None, None, 10000 -1, None)
        self.assertEqual(ec, 5005, 'check fee')

        ec, message = func(None, None, 10000, 10**8-1)
        self.assertEqual(message, 'invalid swapfee parameter! must >= 1 ETP', 'check swapfee')

        ec, message = func(None, None, 10000, 10 ** 8)
        self.assertEqual(message, 'not enough asset amount, unspent = 0, payment = 100')

    def test_2_default(self):
        global AssetName
        message = '{"type":"ETH", "address":"0x0000000000000000000000000000000000000000"}'
        ec, result = mvs_rpc.swap_token(Alice.name, Alice.password, AssetName, 1, message)
        self.assertEqual(ec, 0)

        #result = result['transaction']
        actual_output0 = copy.copy(result['outputs'][0])
        actual_output0.pop('script')
        expect_output0= {u'index': 0, u'value': 0, u'attachment': {u'symbol': AssetName, u'type': u'asset-transfer', u'quantity': 1}, u'address': u'MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh', u'locked_height_range': 0}
        self.assertEqual(actual_output0, expect_output0)

        actual_output1 = copy.copy(result['outputs'][1])
        actual_output1.pop('script')
        expect_output1 = {u'index': 1, u'value': 100000000, u'attachment': {u'type': u'etp'}, u'address': u'MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh', u'locked_height_range': 0}
        self.assertEqual(actual_output1, expect_output1)

        actual_output4 = copy.copy(result['outputs'][4])
        actual_output4.pop('script')
        actual_output4.pop('address')
        expect_output4 = {u'index': 4, u'value': 0, u'attachment': {u'content': message, u'type': u'message'}, u'locked_height_range': 0}
        self.assertEqual(actual_output4, expect_output4)
        Alice.mining()

    def test_3_fromdid(self):
        global AssetName
        message = '{"type":"ETH", "address":"0x0000000000000000000000000000000000000001"}'
        ec, result = mvs_rpc.swap_token(Alice.name, Alice.password, AssetName, 1, message, from_=Alice.did_symbol)
        self.assertEqual(ec, 0)
        #result = result['transaction']

        actual_output0 = copy.copy(result['outputs'][0])
        actual_output0.pop('script')
        expect_output0 = {u'index': 0, u'value': 0, u'attachment': {u'from_did': Alice.did_symbol, u'to_did': u'', u'type': u'asset-transfer', u'symbol': AssetName, u'quantity': 1}, u'address': u'MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh', u'locked_height_range': 0}
        self.assertEqual(actual_output0, expect_output0)

        actual_output1 = copy.copy(result['outputs'][1])
        actual_output1.pop('script')
        expect_output1 = {u'index': 1, u'value': 100000000, u'attachment': {u'from_did': Alice.did_symbol, u'to_did': u'', u'type': u'etp'}, u'address': u'MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh', u'locked_height_range': 0}
        self.assertEqual(actual_output1, expect_output1)
        Alice.mining()

    def test_4_fromaddress(self):
        pass

    def test_5_change(self):
        global AssetName
        message = '{"type":"ETH", "address":"0x0000000000000000000000000000000000000002"}'
        ec, result = mvs_rpc.swap_token(Alice.name, Alice.password, AssetName, 1, message, change=Alice.addresslist[1])
        self.assertEqual(ec, 0)
        #result = result['transaction']

        self.assertEqual([result['outputs'][2]['address'], result['outputs'][3]['address']], [Alice.addresslist[1]]*2)

        #charge the fee for tx
        Alice.send_etp(Alice.addresslist[1], 10000)
        Alice.mining()
        # send the asset back to the main address
        ec, result = mvs_rpc.get_addressasset(Alice.addresslist[1])
        #result = result['assets']
        self.assertEqual(ec, 0)
        quantity = [i["quantity"] for i in result if i["symbol"] == AssetName][0]
        Alice.send_asset_from(Alice.addresslist[1], Alice.mainaddress(), quantity, AssetName)
        Alice.mining()

    def test_6_swapfee(self):
        global AssetName
        message = '{"type":"ETH", "address":"0x0000000000000000000000000000000000000002"}'
        ec, result = mvs_rpc.swap_token(Alice.name, Alice.password, AssetName, 1, message, swapfee=10**8 + 123456)
        self.assertEqual(ec, 0)
        #result = result['transaction']

        actual_output1 = copy.copy(result['outputs'][1])
        actual_output1.pop('script')
        expect_output1 = {u'index': 1, u'value': 100123456, u'attachment': {u'type': u'etp'}, u'address': u'MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh', u'locked_height_range': 0}
        self.assertEqual(actual_output1, expect_output1)
        Alice.mining()
