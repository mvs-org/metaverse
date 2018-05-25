import time
from utils import common
import MOCs
from TestCase.MVSTestCase import *

class TestAssetAfterDidModified(MVSTestCaseBase):

    def test_0_scenario_did_modified(self):
        Alice.ensure_balance()

        # create did
        #
        fst_did_symbol = Alice.did_symbol
        fst_did_address = Alice.mainaddress()

        result, message = mvs_rpc.sendfrom(Alice.name, Alice.password, fst_did_address, Alice.didaddress(), 2 * 10 ** 8)
        assert (result == 0)

        Alice.mining()
        Alice.mining()
        result, addresslist = mvs_rpc.list_addresses(Alice.name, Alice.password)

        used_addresses = []
        ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
        assert (ec == 0)
        if message['dids']:
            dids = [MOCs.Did.init(i) for i in message["dids"] if i]
            used_addresses = [did.address for did in dids if did]

        addresslist =  list(set(addresslist) ^ set(used_addresses))
        length = len(addresslist)
        assert(length > 2)

        snd_did_address = addresslist[length - 1]
        snd_did_symbol = u"modifydiid." + common.get_random_str()

        result, message = mvs_rpc.sendfrom(Alice.name, Alice.password, fst_did_address, snd_did_address, 12 * 10 ** 8)
        assert (result == 0)
        Alice.mining()

        ec, message = Alice.issue_did(snd_did_address, snd_did_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # prepare address
        #
        trd_address = addresslist[length - 2]

        result, message = mvs_rpc.sendfrom(Alice.name, Alice.password, fst_did_address, trd_address, 12 * 10 ** 8)
        assert (result == 0)
        Alice.mining()

        # create asset and cert
        #
        domain_symbol, fst_asset_symbol = Alice.create_random_asset(did_symbol=snd_did_symbol, secondary=-1)
        Alice.mining()

        # modify address of did
        ec, message = mvs_rpc.modify_did(Alice.name, Alice.password, trd_address, snd_did_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test send asset
        ec, message = mvs_rpc.didsend_asset(Alice.name, Alice.password, snd_did_symbol, fst_asset_symbol, 3000)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password,
            to_did=fst_did_symbol, symbol=fst_asset_symbol, volume=3000)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test issue cert
        naming_cert_symbol = domain_symbol + ".NAMING." + common.get_random_str()
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, fst_did_symbol, naming_cert_symbol, "naming")
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test transfer cert
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, fst_did_symbol, fst_asset_symbol, 'issue')
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test issue new asset
        #
        snd_asset_symbol = domain_symbol + ".ASSEET." + common.get_random_str()
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, snd_asset_symbol,
            volume=8000000, issuer=fst_did_symbol, rate=-1)
        self.assertEqual(ec, code.success, message)

        ec, message = mvs_rpc.issue_asset(Alice.name, Alice.password, snd_asset_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password,
            to_did=fst_did_symbol, symbol=snd_asset_symbol, volume=3000)
        self.assertEqual(ec, code.success, message)
        Alice.mining()
