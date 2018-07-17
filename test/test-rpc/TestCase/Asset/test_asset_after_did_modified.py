import time
from utils import common
import MOCs
from TestCase.MVSTestCase import *

class TestAssetAfterDidModified(MVSTestCaseBase):

    def test_0_scenario_did_modified(self):

        # prepare address
        #
        Zac.mining()
        Zac.mining()
        Zac.mining()
        result, addresslist = mvs_rpc.list_addresses(Zac.name, Zac.password)

        used_addresses = []
        ec, message = mvs_rpc.list_dids(Zac.name, Zac.password)
        self.assertEqual(ec, code.success, message)

        if message['dids']:
            dids = [MOCs.Did.init(i) for i in message["dids"] if i]
            used_addresses = [did.address for did in dids if did]

        addresslist =  list(set(addresslist) ^ set(used_addresses))
        length = len(addresslist)
        assert(length > 3)

        # create first did
        fst_did_address = addresslist[length - 1]
        fst_did_symbol = u"zacfirstdiid." + common.get_random_str()

        result, message = mvs_rpc.send(Alice.name, Alice.password, fst_did_address, 30 * 10 ** 8)
        self.assertEqual(result, code.success, message)
        Zac.mining()

        ec, message = Zac.register_did(fst_did_address, fst_did_symbol)
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # create second did
        snd_did_address = addresslist[length - 2]
        snd_did_symbol = u"zacmodifydiid." + common.get_random_str()

        result, message = mvs_rpc.send(Alice.name, Alice.password, snd_did_address, 15 * 10 ** 8)
        self.assertEqual(result, code.success, message)
        Zac.mining()

        ec, message = Zac.register_did(snd_did_address, snd_did_symbol)
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # prepare third address
        #
        trd_address = addresslist[length - 3]

        result, message = mvs_rpc.send(Alice.name, Alice.password, trd_address, 15 * 10 ** 8)
        self.assertEqual(result, code.success, message)
        Zac.mining()

        # create asset and cert
        #
        domain_symbol, fst_asset_symbol = Zac.create_random_asset(did_symbol=snd_did_symbol, secondary=-1)
        Zac.mining()

        # change address of did
        ec, message = mvs_rpc.change_did(Zac.name, Zac.password, trd_address, snd_did_symbol)
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # test send asset
        ec, message = mvs_rpc.didsend_asset(Zac.name, Zac.password, snd_did_symbol, fst_asset_symbol, 3000)
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # test secondary_issue
        ec, message = mvs_rpc.secondary_issue(Zac.name, Zac.password,
            to_did=fst_did_symbol, symbol=fst_asset_symbol, volume=3000)
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # test issue cert
        naming_cert_symbol = domain_symbol + ".NAMING." + common.get_random_str()
        ec, message = mvs_rpc.issue_cert(Zac.name, Zac.password, fst_did_symbol, naming_cert_symbol, "naming")
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # test transfer cert
        ec, message = mvs_rpc.transfer_cert(Zac.name, Zac.password, fst_did_symbol, fst_asset_symbol, 'issue')
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # test issue new asset
        #
        snd_asset_symbol = domain_symbol + ".ASSEET." + common.get_random_str()
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, snd_asset_symbol,
            volume=8000000, issuer=fst_did_symbol, rate=-1)
        self.assertEqual(ec, code.success, message)

        ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password, snd_asset_symbol)
        self.assertEqual(ec, code.success, message)
        Zac.mining()

        # test secondary_issue
        ec, message = mvs_rpc.secondary_issue(Zac.name, Zac.password,
            to_did=fst_did_symbol, symbol=snd_asset_symbol, volume=3000)
        self.assertEqual(ec, code.success, message)
        Zac.mining()
