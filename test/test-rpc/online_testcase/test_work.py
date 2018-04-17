import os, sys
sys.path.append('..')
import unittest
import utils.mvs_rpc as mvs_rpc
import utils.common as common
import binascii
from ethereum.pow.ethpow import mine


class TestWork(unittest.TestCase):
    account = "mvs_test"
    password = "tkggfngu"
    mnemonic = "alarm void rib clean stay suggest govern stove engage fury will mutual baby funny empower raccoon cement federal raise rocket fashion merit design cube".split()
    light_wallet_keystore = os.path.abspath('../resource/keystore/mvs_tkggfngu.json')
    addresses = [
        "MTU7t8AZRazN2t5p3dHRF9k6b2ibisxZxi",
        "MSoq2Ec8icWX9Ur7pSqK8mfi14XxRwPBPC",
        "MMfZsZckHJ4qskhfhzAqbPTnWfeB6bthV7",
        "MCbAqMwGMeWWpbvmfjQ8ETy6rSgiApgTLc",
        "M8QsckJYcy7FdQpzswJt49YFXkzCgqRQRw",
        "MJo4yAyfd8YTsmBKFcdmPKm9p4PcrB7gmo",
        "MFK1qPdnRs3CR6QNMBz8Fau1ns1Vc1Usi9",
        "MEFFdkvbcjMo7jh3oe2eSFoB1e217Aut6U",
        "MA1Wo8UmTxxErUfUSjrVd8zKWumfzbLnSw",
        "MBZFKYRwXhb6QrWpmxQcALJWBZfmMzMFUW"
    ]

    def setUp(self):
        '''
        '''
        result, message = mvs_rpc.import_keyfile(
            self.account,
            self.password,
            self.light_wallet_keystore)
        self.assertEqual(result, True, message)

        result, message = mvs_rpc.set_miningaccount(
            self.account,
            self.password,
            self.addresses[0]
        )
        self.assertEqual(result, True, message)

    def tearDown(self):
        result, message = mvs_rpc.delete_account(
            self.account,
            self.password,
            self.mnemonic[-1])
        self.assertEqual(result, True, message)

    def mine(self):
        header_hash, seed_hash, boundary = mvs_rpc.eth_get_work()
        height, difficulty = mvs_rpc.get_info()

        # difficulty = 100

        rounds = 100
        nonce = 0
        while True:
            bin_nonce, mixhash = mine(block_number=height + 1, difficulty=difficulty, mining_hash=header_hash,
                                      rounds=rounds, start_nonce=nonce)
            if bin_nonce:
                break
            nonce += rounds
        return bin_nonce, '0x'+common.toString(header_hash), '0x'+common.toString(mixhash)

    def test_eth_submitwork(self):

        #print common.toString(bin_nonce)
        #print common.toString(mixhash)
        bin_nonce, header_hash, mix_hash = self.mine()

        result, message = mvs_rpc.eth_submit_work('0x'+common.toString(bin_nonce), header_hash, mix_hash)
        self.assertEqual(result, True, message)

    def test_eth_submitwork_check(self):
        '''
        check nonce startswith 0x
        '''
        bin_nonce, header_hash, mix_hash = self.mine()

        result, message = mvs_rpc.eth_submit_work(common.toString(bin_nonce), header_hash, mix_hash)
        self.assertEqual(result, False, message)
        self.assertEqual(message[u'message'], u'nonce should start with "0x" for eth_submitWork', message)

    def test_submitwork(self):
        '''
         nonce is not startswith 0x
        '''
        bin_nonce, header_hash, mix_hash = self.mine()

        nonce = int(common.toString(bin_nonce), 16)
        str_nonce = hex(nonce ^ 0x6675636b6d657461)[2:]
        #print str_nonce
        result, message = mvs_rpc.submit_work(str_nonce, header_hash, mix_hash)
        self.assertEqual(result, True, message)