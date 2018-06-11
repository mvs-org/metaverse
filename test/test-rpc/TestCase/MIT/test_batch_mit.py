import time
import MOCs
from utils import common
from TestCase.MVSTestCase import *

class TestRegisterMIT(MVSTestCaseBase):

    def test_0_boundary(self):
        test_symbol = common.get_random_str()

        # check symbol length
        mits=["X"*61 + ":invalid"]
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.asset_symbol_length_exception, message)

        # check invalid char in symbol
        spec_char_lst = "`~!#$%^&*()+[{]}\\|;'\",<>/?"
        for char in spec_char_lst:
            mits=[test_symbol + char + ":invalid"]
            ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
            self.assertEqual(ec, code.asset_symbol_name_exception, message)

        # check content length
        mits=[test_symbol + ":" + "X"*257]
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.argument_size_invalid_exception, message)

        # check duplicate symbols
        mits=[test_symbol + ":" + "M"*256, test_symbol + ":" + "Duplicate symbol"]
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.asset_symbol_existed_exception, message)

        # check empty symbol
        mits=[]
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.argument_legality_exception, message)

        # check to did not exist
        mits=[test_symbol + ":" + "M"*256]
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol + "1", mits=mits)
        self.assertEqual(ec, code.did_symbol_notfound_exception, message)

        # check to did not owned
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Bob.did_symbol, mits=mits)
        self.assertEqual(ec, code.address_dismatch_account_exception, message)

        # check symbol already exist
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.success, message)
        Alice.mining()

        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.asset_symbol_existed_exception, message)

    def test_1_register_mits(self):
        # set max_paramters in Mongoose.hpp to 208
        max_mit_count = 100

        mits[:] = []
        for i in range(0, max_mit_count):
            mits.append("{}@{}:content of {}".format(common.get_random_str(), i, i))
        ec, message = mvs_rpc.register_mit(Alice.name, Alice.password, Alice.did_symbol, mits=mits)
        self.assertEqual(ec, code.success, message)
        Alice.mining()