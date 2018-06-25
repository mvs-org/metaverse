from utils import common
from TestCase.MVSTestCase import *

class TestBlock(MVSTestCaseBase):
    need_mine = False
    roles = ()
    def test_0_getblock(self):
        ec, message = mvs_rpc.get_blockheader()
        self.assertEqual(ec, 0, message)

        hash = message["hash"]
        height = message["number"]

        ec, message_by_hash = mvs_rpc.get_block(hash)
        self.assertEqual(ec, 0, message)

        ec, message_by_height = mvs_rpc.get_block(height)
        self.assertEqual(ec, 0, message)

        self.assertEqual(message_by_hash, message_by_height)

        # height error
        ec, message_by_height = mvs_rpc.get_block(height+1)
        self.assertEqual(ec, 5101, message)

    def test_1_getblockheader(self):
        ec, message = mvs_rpc.get_blockheader()
        self.assertEqual(ec, 0, message)
        # check keys
        expect_keys = ["bits", "hash", "merkle_tree_hash", "mixhash", "nonce", "number", "previous_block_hash", "time_stamp", "transaction_count", "version"]
        expect_keys.sort()

        actual_keys = message.keys()
        actual_keys.sort()
        self.assertEqual(actual_keys, expect_keys)

        hash = message["hash"]
        height = message["number"]

        # by hash
        ec, message_by_hash = mvs_rpc.get_blockheader(hash=hash)
        self.assertEqual(ec, 0)
        self.assertEqual(message, message_by_hash)

        # by height
        ec, message_by_height = mvs_rpc.get_blockheader(height=height)
        self.assertEqual(ec, 0)
        self.assertEqual(message, message_by_height)

        # check previous_block_hash
        previous_block_hash = message["previous_block_hash"]
        ec, previous_block = mvs_rpc.get_blockheader(hash=previous_block_hash)
        self.assertEqual(ec, 0)
        self.assertEqual(previous_block["number"], height-1)


