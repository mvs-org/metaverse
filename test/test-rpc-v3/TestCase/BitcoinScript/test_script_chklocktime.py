from TestCase.MVSTestCase import *
from utils import compile, set_rawtx
from binascii import b2a_hex, a2b_hex
import struct

redeem_script = '''
%(Height)s OP_CHECKLOCKTIMEVERIFY OP_DROP %(Alice)s OP_CHECKSIG
'''

spent_script = '''
%(SIG)s
OP_PUSHDATA1
%(redeem)s
'''

from TestCase.MVSTestCase import *

class TestChkLocktime(MVSTestCaseBase):
    need_mine = False
    def test_0_ChkLocktime(self):
        import pdb
        pdb.set_trace()
        ec, (height, _) = mvs_rpc.get_info()
        self.assertEqual(ec, 0)

        ec, pk = mvs_rpc.get_publickey(Alice.name, Alice.password, Alice.mainaddress())
        self.assertEqual(ec, 0)

        lk_height = height + 10

        ht = struct.pack("<L", lk_height)
        bin_script = compile.script_to_hex(redeem_script % {
            'Alice' : pk,
            'Height' : ht.hex(),
        })

        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, bin_script)
        self.assertEqual(ec, 0 , message)
        p2sh = message['address']

        txhash = Alice.send_etp(p2sh, 10**8)
        Alice.mining()

        ec, rawtx = mvs_rpc.create_rawtx(0, [], {Zac.mainaddress():5 * 10**7}, utxos=[(txhash,0,0xFFFFFFFF)], mychange=p2sh, locktime=lk_height)
        self.assertEqual(ec, 0, rawtx)

        ec, sigs = mvs_rpc.sign_rawtx(Alice.name, Alice.password, rawtx)
        self.assertEqual([pk], list( sigs['0'].keys() ), sigs)

        sig = sigs['0'][pk]

        bin_spent_script = compile.script_to_hex( spent_script % {'SIG':sig, "PK": pk, "redeem" : bin_script} )
        tx = set_rawtx.Transaction.parse_rawtx(rawtx)
        tx.inputs[0].script = a2b_hex(bin_spent_script)

        Alice.mining(8)
        ec, message = mvs_rpc.send_rawtx(tx.to_rawtx())
        self.assertEqual(ec, 5304, message)

        Alice.mining(1)
        ec, message = mvs_rpc.send_rawtx(tx.to_rawtx())
        self.assertEqual(ec, 0, message)


