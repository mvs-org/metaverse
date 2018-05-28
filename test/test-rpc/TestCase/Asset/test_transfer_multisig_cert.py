import time
from utils import common
from TestCase.MVSTestCase import *

class TestTransferMultisigCert(MVSTestCaseBase):

    def test_0_transfer_multisig_cert(self):
        # Alice create asset
        domain_symbol, asset_symbol = Alice.create_random_asset(secondary=-1)
        Alice.mining()

        # create multisig
        #
        description = "Alice & Zac's multi-sig address"

        multisig_address = Alice.new_multisigaddress(description, [Zac], 2)
        multisig_address2 = Zac.new_multisigaddress(description, [Alice], 2)
        self.assertEqual(multisig_address, multisig_address2, "multisig addresses dismatch.")

        # send etp to multisig_address
        #
        result, message = mvs_rpc.sendfrom(Alice.name, Alice.password,
            Alice.mainaddress(), multisig_address, 3 * 10 ** 8)
        assert (result == 0)
        Alice.mining()

        # issue did to multisig_address
        #
        multisig_did_symbol = "Multisig" + common.get_random_str()
        ec, tx = mvs_rpc.issue_did(Alice.name, Alice.password, multisig_address, multisig_did_symbol)
        self.assertEqual(ec, code.success, tx)

        # sign multisig rawtx
        ec, tx2 = mvs_rpc.sign_multisigtx(Zac.name, Zac.password, tx, True)
        self.assertEqual(ec, 0, tx2)
        Alice.mining()

        # transfer cert to multisig_did_symbol
        #
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, multisig_did_symbol, asset_symbol, 'issue')
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # check cert
        certs = Zac.get_addressasset(multisig_address, True);
        self.assertGreater(len(certs), 0, "not cert found at " + multisig_address)
        exist_symbols = filter(lambda a: a.symbol == asset_symbol and a.cert == "issue", certs)
        self.assertEqual(len(exist_symbols), 1, "not cert found at " + multisig_address)

        # transfer cert to Cindy
        #
        ec, tx = mvs_rpc.transfer_cert(Alice.name, Alice.password, Cindy.did_symbol, asset_symbol, 'issue')
        self.assertEqual(ec, code.success, tx)

        # sign multisig rawtx
        ec, tx2 = mvs_rpc.sign_multisigtx(Zac.name, Zac.password, tx, True)
        self.assertEqual(ec, 0, tx2)
        Alice.mining()

        # check cert
        certs = Cindy.get_addressasset(Cindy.didaddress(), True);
        self.assertGreater(len(certs), 0, "not cert found at " + Cindy.didaddress())
        exist_symbols = filter(lambda a: a.symbol == asset_symbol and a.cert == "issue", certs)
        self.assertEqual(len(exist_symbols), 1, "not cert found at " + Cindy.didaddress())
