import time
import MOCs
from utils import common
from TestCase.MVSTestCase import *


class TestRegisterMIT(MVSTestCaseBase):

    def test_0_boundary(self):

        test_symbol = common.get_random_str()

        # account password error
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password + '1', Alice.did_symbol, test_symbol)
        self.assertEqual(ec, 1000, message)

        # check symbol length
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, "X" * 61)
        self.assertEqual(ec, code.asset_symbol_length_exception, message)

        # check invalid char in symbol
        spec_char_lst = "`~!#$%^&*()+[{]}\\|;:'\",<>/?"
        for char in spec_char_lst:
            ec, message = mvs_rpc.register_mit(
                Alice.name, Alice.password, Alice.did_symbol, test_symbol + char)
            self.assertEqual(ec, code.asset_symbol_name_exception, message)

        # check content length
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, test_symbol, "X" * 257)
        self.assertEqual(ec, code.argument_size_invalid_exception, message)

        # check to did not exist
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol + "1", test_symbol)
        self.assertEqual(ec, code.did_symbol_notfound_exception, message)

        # check to did not owned
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Bob.did_symbol, test_symbol)
        self.assertEqual(ec, code.address_dismatch_account_exception, message)

        # check symbol already exist
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, test_symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, test_symbol)
        self.assertEqual(ec, code.asset_symbol_existed_exception, message)

        # check max length of content 256
        test_symbol = common.get_random_str()
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, test_symbol, "X" * 256)
        self.assertEqual(ec, code.success, message)

    def test_1_register_mit(self):
        symbol = ("MIT." + common.get_random_str()).upper()
        content = "MIT of Alice: " + symbol
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, symbol, content)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test list_mits with account
        ec, message = Alice.list_mits(Alice.name, Alice.password)
        self.assertEqual(ec, code.success, message)

        mits = message
        self.assertGreater(len(mits), 0)
        found_mits = filter(lambda a: a["symbol"] == symbol, mits)
        self.assertEqual(len(found_mits), 1)

        mit = found_mits[0]
        self.assertEqual(mit["symbol"], symbol)
        self.assertEqual(mit["content"], content)
        self.assertEqual(mit["status"], "registered")

        # test list_mits without account
        ec, message = Alice.list_mits()
        self.assertEqual(ec, code.success, message)

        mits = message
        self.assertGreater(len(mits), 0)
        found_mits = filter(lambda a: a["symbol"] == symbol, mits)
        self.assertEqual(len(found_mits), 1)

        mit = found_mits[0]
        self.assertEqual(mit["symbol"], symbol)
        self.assertEqual(mit["content"], content)
        self.assertEqual(mit["status"], "registered")

        # test get_mit with symbol
        ec, message = Alice.get_mit(symbol)
        self.assertEqual(ec, code.success, message)
        self.assertEqual(message["symbol"], symbol)
        self.assertEqual(message["content"], content)
        self.assertEqual(message["status"], "registered")

        # test get_mit without symbol
        ec, message = Alice.get_mit()
        self.assertEqual(ec, code.success, message)

        mits = message
        self.assertGreater(len(mits), 0)
        found_mits = filter(lambda a: a == symbol, mits)
        self.assertEqual(len(found_mits), 1)

    def test_2_transfer_mit(self):
        symbol = ("MIT." + common.get_random_str()).upper()
        content = "MIT of Alice: " + symbol

        # not enough mit
        ec, message = Alice.transfer_mit(Bob.did_symbol, symbol)
        self.assertEqual(ec, code.asset_lack_exception, message)

        # register mit
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, symbol, content)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # transfer mit
        ec, message = Alice.transfer_mit(Bob.did_symbol, symbol)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test get_mit with symbol
        ec, message = Bob.get_mit(symbol)
        self.assertEqual(ec, code.success, message)
        self.assertEqual(message["symbol"], symbol)
        self.assertEqual(message["content"], content)
        self.assertEqual(message["status"], "registered")

        # test get_mit with symbol and history
        ec, message = Bob.get_mit("", True)
        self.assertEqual(ec, code.argument_legality_exception, message)

        ec, message = Bob.get_mit(symbol, True, 0, 100)
        self.assertEqual(ec, code.argument_legality_exception, message)

        ec, message = Bob.get_mit(symbol, True, 1, 101)
        self.assertEqual(ec, code.argument_legality_exception, message)

        # success
        ec, message = Bob.get_mit(symbol, True)
        self.assertEqual(ec, code.success, message)

        mits = message
        self.assertGreater(len(mits), 0)
        found_mits = filter(lambda a: a["symbol"] == symbol, mits)
        self.assertEqual(len(found_mits), 2)

        mit = found_mits[0]
        self.assertEqual(mit["symbol"], symbol)
        self.assertEqual(mit["status"], "transfered")

        # root
        mit = found_mits[1]
        self.assertEqual(mit["symbol"], symbol)
        self.assertEqual(mit["status"], "registered")

        # not enough mit
        ec, message = Alice.transfer_mit(Bob.did_symbol, symbol)
        self.assertEqual(ec, code.asset_lack_exception, message)

    def test_3_burn_mit(self):
        symbol = ("MIT." + common.get_random_str()).upper()
        content = "MIT of Alice: " + symbol

        # register mit
        ec, message = mvs_rpc.register_mit(
            Alice.name, Alice.password, Alice.did_symbol, symbol, content)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # burn mit
        ec, message = mvs_rpc.burn(
            Alice.name, Alice.password, symbol, is_mit=True)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        # test get_mit with symbol
        ec, message = Alice.get_mit(symbol, trace=True)
        self.assertEqual(ec, code.success, message)
        self.assertEqual(len(message), 2)
        self.assertEqual(message[0]["to_did"], "BLACKHOLE")
