from TestCase.MVSTestCase import *
from utils import compile, set_rawtx
from binascii import b2a_hex, a2b_hex

# Alice and Bob multisig, or Alice with 10 blocks
hex_script= "6352210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d112102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c6484657352ae67040a000000b375210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d11ac68"
p2sh_address = "3B95d2Nm8ZBMiENnxkTHRaHE8RhUxhd9KK"

multisig_unlock_branch = '''
OP_0
%(Alice)s
%(Bob)s
OP_1
OP_PUSHDATA1
%(redeem)s
'''

alice_with_10_block_branch = '''
%(Alice)s
OP_0
OP_PUSHDATA1
%(redeem)s
'''

class TestScript(MVSTestCaseBase):
    def test_0_spent_by_multisig(self):
        txhash = Alice.send_etp(p2sh_address, 10**8)
        Alice.mining()

        # nsequence not set -- spent by multisig branch
        ec, message = mvs_rpc.create_rawtx(0, [], {Zac.mainaddress():5*10**7}, mychange=p2sh_address, utxos=[(txhash, 0, 0xFFFFFFFF)])
        self.assertEqual(ec, 0, message)

        rawtx = message
        dict_sig = {} # name : edsig
        for role in (Alice, Bob):
            # import script
            ec, message = mvs_rpc.import_address(role.name, role.password, hex_script, "test_0_spent_by_multisig")
            self.assertEqual(ec, 0, message)
            self.assertEqual(message['address'], p2sh_address)

            ec, message = mvs_rpc.sign_rawtx(role.name, role.password, rawtx)
            self.assertEqual(ec, 0, message)

            edsig = message["0"][role.get_publickey(role.mainaddress())]
            dict_sig[role.name] = edsig

        unlock_script_para = {'redeem':hex_script}
        unlock_script_para.update(dict_sig)
        unlock_script = multisig_unlock_branch % unlock_script_para
        hex_unlock_script = compile.script_to_hex(unlock_script)

        tx = set_rawtx.Transaction.parse_rawtx(rawtx)
        tx.inputs[0].script = a2b_hex(hex_unlock_script)
        rawtx = tx.to_rawtx()
        ec, message = mvs_rpc.send_rawtx(rawtx)
        self.assertEqual(ec, 0, message)

        # ------------------    devide line     -----------------------------------
        Alice.mining()

        output_txhash = message
        # spent by sequence lock branch
        ec, message = mvs_rpc.create_rawtx(0, [], {Zac.mainaddress(): 25 * 10 ** 6},
                                           mychange=p2sh_address, utxos=[(output_txhash, 1, 10)])
        self.assertEqual(ec, 0, message)

        rawtx = message
        dict_sig = {}  # name : edsig
        for role in (Alice, ):
            ec, message = mvs_rpc.sign_rawtx(role.name, role.password, rawtx)
            self.assertEqual(ec, 0, message)

            edsig = message["0"][role.get_publickey(role.mainaddress())]
            dict_sig[role.name] = edsig

        unlock_script_para = {'redeem': hex_script}
        unlock_script_para.update(dict_sig)
        unlock_script = alice_with_10_block_branch % unlock_script_para
        hex_unlock_script = compile.script_to_hex(unlock_script)

        tx = set_rawtx.Transaction.parse_rawtx(rawtx)
        tx.inputs[0].script = a2b_hex(hex_unlock_script)
        rawtx = tx.to_rawtx()
        ec, message = mvs_rpc.send_rawtx(rawtx)
        self.assertEqual(ec, 5304, message) # locked, need mine 10 blocks

        Alice.mining(8)
        ec, message = mvs_rpc.send_rawtx(rawtx)
        self.assertEqual(ec, 5304, message)

        Alice.mining(1)
        ec, message = mvs_rpc.send_rawtx(rawtx)
        self.assertEqual(ec, 0, message)
