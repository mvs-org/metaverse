import os, sys
sys.path.append('..')
import unittest
import utils.mvs_rpc as mvs_rpc
import utils.common as common
import utils.cryptojs as cryptojs
import json

class TestDumpKeyfile(unittest.TestCase):
    account = "mvs_test"
    password = "tkggfngu"
    mnemonic = "alarm void rib clean stay suggest govern stove engage fury will mutual baby funny empower raccoon cement federal raise rocket fashion merit design cube".split()
    light_wallet_keystore = os.path.abspath('../resource/keystore/mvs_tkggfngu.json')
    addresses = [
        "02f1f814de64580cf0ab8473ab992bc329f8bb7044bd1d88e2a190f9e1307e774f",
        "03de7fcb33c3d1c66550a925b929891331e6847616f1820c0f299f022e9a12836e",
        "03bd5b4209249c89030c1aca3450b6e0e14361e685b39419dc2f8853f6a575edb4",
        "03bbb9609e633583ca6ac03434a3f64c1ea9b3a8aa964b5f9deeebb525c978abd4",
        "023583567be0cd9db3d185c6acdd2c45799ca011df3f59c9e46130ab21aa878d95",
        "033b713146eb83f862ab6b8cbbca9490e4dacbca80679df5060c55b7705c5b839f",
        "03dae188dcfa86bc0850e4428c404fed4fd7f094fb763c8320a42135dcae72c628",
        "03cd45edb86a44dca3cb7f30a3ca4b59e26b424cd02bc152fecbb80580c657398a",
        "033b768c5e2fdadb9b3a64e776ee41166d2fdfd7176d9263497157f98229201a82",
        "02e910d86d70bb3b9f126e550a7891d9cffc67a6ab9b68616c571f76e7cf5e86f0",
    ]

    others_addresses = ["0200e5782241ce24af725f2e823dfcc325101cec604e422566ba5ce4ca4bd5bc8c", "03046e1c2777cfa064932a2f4c12f8dd307c1702c9cd77d14c48daf134627e355e"]

    def setUp(self):
        '''
        create account by ImportLightWalletKeyfile
        :return:
        '''
        result, message = mvs_rpc.import_keyfile(
            self.account,
            self.password,
            self.light_wallet_keystore)
        self.assertEqual(result, True, message)

    def tearDown(self):
        '''
        remove the account
        :return:
        '''
        result, message = mvs_rpc.delete_account(
            self.account,
            self.password,
            self.mnemonic[-1])
        self.assertEqual(result, True, message)

    def testDumpKeyFile(self):
        #1. create multisig address
        import datetime
        nowTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        description = 'making test at' + nowTime
        result, message = mvs_rpc.getnew_multisig(
            self.account,
            self.password,
            description,
            self.addresses[0],
            self.others_addresses,
            2)
        self.assertEqual(result, True, message)

        #2. dump keyfile
        keyfile = os.path.abspath("./fullwallet_keystore.json")
        common.remove_file(keyfile)
        result, message = mvs_rpc.dump_keyfile(
            self.account,
            self.password,
            self.mnemonic[-1],
            keyfile)
        self.assertEqual(result, True, message)
        self.assertEqual(os.path.exists(keyfile), True, keyfile + " not exists!")

        #3. check encrypted-mnemonic in keyfile
        f = open(keyfile)
        keyfile_content = json.load(f)
        f.close()

        mnemonic = cryptojs.AES_CBC_decrypt(keyfile_content['mnemonic'], self.password)
        self.assertEqual(mnemonic, '"' + " ".join(self.mnemonic) + '"', "mnemonic not match")

        #4. delete the account
        result, message = mvs_rpc.delete_account(
            self.account,
            self.password,
            self.mnemonic[-1])
        self.assertEqual(result, True, message)
        # ensure the account has been deleted
        result, message = mvs_rpc.get_account(
            self.account,
            self.password,
            self.mnemonic[-1])
        self.assertEqual(result, False, message)

        #5. recover the account by FullWalletKeyfile
        result, message = mvs_rpc.import_keyfile(
            self.account,
            self.password,
            keyfile)
        self.assertEqual(result, True, message)

        common.remove_file(keyfile)

        #6. check public address and multisig address
        result, message = mvs_rpc.list_multisig(
            self.account,
            self.password)
        self.assertEqual(result, True, message)
        multisig = message['multisig']
        self.assertEqual(len(multisig), 1, message)
        self.assertEqual(multisig[0]['self-publickey'], self.addresses[0])
        self.assertEqual(multisig[0]['description'], description)
        all_addresses = self.others_addresses + self.addresses[:1]
        all_addresses.sort()
        self.assertEqual(multisig[0]['public-keys'], all_addresses)
        self.assertEqual(multisig[0]['m'], '2')
        self.assertEqual(multisig[0]['n'], '3')




