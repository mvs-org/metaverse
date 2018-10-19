from TestCase.MVSTestCase import *
from utils.set_rawtx import Transaction

def prepare_rawtx():
    ec, rawtx = mvs_rpc.create_rawtx(0, [Alice.mainaddress()], {Bob.mainaddress(): 10**7})
    assert (ec, 0)
    return rawtx

class TestTxSequence(MVSTestCaseBase):
    def test_0_enter_txpool_by_height(self):
        rawtx = prepare_rawtx()
        t = Transaction.parse_rawtx(rawtx)

        ec, message = mvs_rpc.gettx( t.inputs[0].utxo_hash[::-1].hex() )
        self.assertEqual(ec, 0, message)
        prev_height = message['height']

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        height = message[0]

        # eg: prev_height=1000,  height=1010
        # sequence = 11 -> ok
        # sequence = 12 -> nok
        for sequence, expect_ec in [(height - prev_height + 2, 5304), (height - prev_height + 1, 0)]:
            t.inputs[0].sequence = sequence
            raw_tx = t.to_rawtx()
            ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password, raw_tx.hex())
            self.assertEqual(ec, 0, message)
            ec, message = mvs_rpc.send_rawtx(message['rawtx'])
            self.assertEqual(ec, expect_ec, message)

    def test_1_enter_txpool_by_time(self):
        rawtx = prepare_rawtx()
        t = Transaction.parse_rawtx(rawtx)

        ec, message = mvs_rpc.gettx( t.inputs[0].utxo_hash[::-1].hex() )
        self.assertEqual(ec, 0, message)
        prev_height = message['height']

        ec, message = mvs_rpc.get_blockheader(prev_height)
        self.assertEqual(ec, 0, message)
        prev_time = message['timestamp']

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        height = message[0]

        # eg: prev_height=1000,  height=1010
        # sequence = 11 -> ok
        # sequence = 12 -> nok
        for sequence, expect_ec in [(height - prev_height + 2, 5304), (height - prev_height + 1, 0)]:
            t.inputs[0].sequence = sequence
            raw_tx = t.to_rawtx()
            ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password, raw_tx.hex())
            self.assertEqual(ec, 0, message)
            ec, message = mvs_rpc.send_rawtx(message['rawtx'])
            self.assertEqual(ec, expect_ec, message)


    def test_1_accept_block(self):
        '''

        :return:
        '''
        pass


class TestTxLocktime(MVSTestCaseBase):
    need_mine = False
    def test_0_enter_txpool(self):
        rawtx = prepare_rawtx()
        t = Transaction.parse_rawtx(rawtx)

        ec, message = mvs_rpc.gettx( t.inputs[0].utxo_hash[::-1].hex() )
        self.assertEqual(ec, 0, message)
        prev_height = message['height']

        self.assertEqual(type(prev_height), int, message)

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        height = message[0]


