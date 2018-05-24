import time
from utils import common
from TestCase.MVSTestCase import *

class TestAssetAfterDidModified(MVSTestCaseBase):
    need_mine = False

    def test_0_scenario_did_modified(self):

        # create did
        #
        fst_did_symbol = Alice.did_symbol
        fst_did_address = Alice.mainaddress()

        snd_did_address = Alice.addresslist[1]
        snd_did_symbol = fst_did_symbol + ".2nd"

        result, message = mvs_rpc.sendfrom(Alice.name, Alice.password, fst_did_address, snd_did_address, 20 * 10 ** 8)
        assert (result == 0)
        Alice.mining()

        ec, message = Alice.issue_did(snd_did_address, snd_did_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # prepare address
        #
        trd_address = Alice.addresslist[2]

        result, message = mvs_rpc.sendfrom(Alice.name, Alice.password, fst_did_address, trd_address, 20 * 10 ** 8)
        assert (result == 0)
        Alice.mining()

        # create asset and cert
        #
        domain_symbol, fst_asset_symbol = Alice.create_random_asset(secondary=-1)
        Alice.mining()

        # modify address of did
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password, trd_address, fst_did_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test send asset
        ec, message = mvs_rpc.didsend_asset(Alice.name, Alice.password, snd_did_symbol, fst_asset_symbol, 3000)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password,
            to_did=snd_did_symbol, symbol=fst_asset_symbol, volume=3000)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test issue cert
        naming_cert_symbol = domain_symbol + ".NAMING." + common.get_timestamp()
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, snd_did_symbol, naming_cert_symbol, "naming")
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test transfer cert
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, snd_did_symbol, fst_asset_symbol, 'issue')
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test issue new asset
        #
        snd_asset_symbol = domain_symbol + ".ASSET." + common.get_timestamp()
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, snd_asset_symbol,
            volume=8000000, issuer=snd_did_symbol, rate=-1)
        self.assertEqual(ec, code.success, message)

        ec, message = mvs_rpc.issue_asset(Alice.name, Alice.password, snd_asset_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password,
            to_did=snd_did_symbol, symbol=snd_asset_symbol, volume=3000)
        self.assertEqual(ec, code.success, message)
        Alice.mining()
