from TestCase.MVSTestCase import *
from utils.set_rawtx import Transaction

def prepare_rawtx():
    ec, rawtx = mvs_rpc.create_rawtx(0, [Alice.mainaddress()], {Bob.mainaddress(): 4 * 10**8})
    assert (ec == 0)
    return rawtx

locktime_unit = 2 ** 5 # 32 senconds for 1 nlocktime
media_block_count = 11

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

    def test_1_enter_txpool_by_height_2sequence(self):
        rawtx = prepare_rawtx()
        t = Transaction.parse_rawtx(rawtx)

        self.assertGreater(len(t.inputs), 1, 'expect more than 2 inputs')

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        height = message[0]

        ec, message = mvs_rpc.gettx( t.inputs[0].utxo_hash[::-1].hex() )
        self.assertEqual(ec, 0, message)
        prev_height_0 = message['height']
        sequence_0 = height - prev_height_0 + 1

        ec, message = mvs_rpc.gettx( t.inputs[1].utxo_hash[::-1].hex() )
        self.assertEqual(ec, 0, message)
        prev_height_1 = message['height']
        sequence_1 = height - prev_height_1 + 1


        for a0, a1, expect in [(0, 1, 5304), (1, 0, 5304), (0, 0, 0)]:
            t.inputs[0].sequence = sequence_0 + a0
            t.inputs[1].sequence = sequence_1 + a1
            raw_tx = t.to_rawtx()
            ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password, raw_tx.hex())
            self.assertEqual(ec, 0, message)
            ec, message = mvs_rpc.send_rawtx(message['rawtx'])
            self.assertEqual(ec, expect, message)

    def get_media_past_time(self, height):
        ret = []
        for i in range(media_block_count):
            ec, message = mvs_rpc.get_blockheader(height=height - i)
            self.assertEqual(ec, 0, message)
            ret.append( message['timestamp'] )
        ret.sort()
        return ret[ int(len(ret)/2) ]

    def test_2_enter_txpool_by_time(self):
        rawtx = prepare_rawtx()
        t = Transaction.parse_rawtx(rawtx)

        ec, message = mvs_rpc.gettx( t.inputs[0].utxo_hash[::-1].hex() )
        self.assertEqual(ec, 0, message)
        prev_height = message['height']

        ec, message = mvs_rpc.get_blockheader(height=prev_height)
        self.assertEqual(ec, 0, message)
        prev_time = message['timestamp']

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        height = message[0]

        curr_time = self.get_media_past_time(height)
        sequence = (curr_time - prev_time) / locktime_unit
        if sequence < 1:
            import time
            time.sleep(locktime_unit)
            Alice.mining(media_block_count)
            curr_time = self.get_media_past_time(height + media_block_count)
            sequence = (curr_time - prev_time) / locktime_unit
        sequence = int(sequence)
        # eg: prev_height=1000,  height=1010
        # sequence = 11 -> ok
        # sequence = 12 -> nok
        for sequence_, expect_ec in [(sequence + 1, 5304), (sequence, 0)]:
            t.inputs[0].sequence = ((1 << 22) | sequence_)
            raw_tx = t.to_rawtx()
            ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password, raw_tx.hex())
            self.assertEqual(ec, 0, message)
            ec, message = mvs_rpc.send_rawtx(message['rawtx'])
            self.assertEqual(ec, expect_ec, message)

class TestTxLocktime(MVSTestCaseBase):
    def test_0_enter_txpool_by_height(self):
        rawtx = prepare_rawtx()
        t = Transaction.parse_rawtx(rawtx)
        for i in range(len(t.inputs)):
            t.inputs[i].sequence = (1 << 31)

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        height = message[0]

        for i,e in [(2, 5304), (1, 5304), (0, 0)]:
            t.locktime = height + i
            raw_tx = t.to_rawtx()
            ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password, raw_tx.hex())
            self.assertEqual(ec, 0, message)
            ec, message = mvs_rpc.send_rawtx(message['rawtx'])
            self.assertEqual(ec, e, message)