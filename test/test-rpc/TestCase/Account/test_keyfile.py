import os
import unittest
import utils.mvs_rpc as mvs_rpc
import utils.common as common
import utils.cryptojs as cryptojs
import json
from Roles import Alice, Bob, Cindy

class TestKeyfile(unittest.TestCase):
    roles = [Alice, Bob, Cindy]
    def setUp(self):
        for role in self.roles:
            result, message = role.create()
            self.assertEqual(result, True, message)

    def tearDown(self):
        for role in self.roles:
            result, message = role.delete()
            self.assertEqual(result, True, message)

    def test_dumpkeyfile(self):
        description = "Alice & Bob & Cindy's multi-sig address"

        multisigaddress = Alice.new_multisigaddress(description, [Bob, Cindy], 2)

        keyfile = os.path.abspath("./fullwallet_keystore.json")
        common.remove_file(keyfile)
        result, message = Alice.dump_keyfile(keyfile)
        self.assertEqual(result, True, message)
        self.assertEqual(os.path.exists(keyfile), True, keyfile + " not exists!")

        f = open(keyfile)
        keyfile_content = json.load(f)
        f.close()

        mnemonic = cryptojs.AES_CBC_decrypt(keyfile_content['mnemonic'], Alice.password)
        self.assertEqual(mnemonic, '"' + " ".join(Alice.mnemonic) + '"', "mnemonic not match")

        self.assertEqual(keyfile_content['multisigs'][0]['d'], description, "decription not match!")
        self.assertEqual(keyfile_content['multisigs'][0]['s'], Alice.get_publickey(Alice.mainaddress()))

        #delete account
        Alice.delete()
        #recover the account by FullWalletKeyfile
        result, message = mvs_rpc.import_keyfile(Alice.name, Alice.password, keyfile)
        self.assertEqual(result, True, message)

        common.remove_file(keyfile)

        result, message = mvs_rpc.list_multisig(Alice.name, Alice.password)
        self.assertEqual(result, True, message)
        multisig = message['multisig']
        self.assertEqual(len(multisig), 1, message)
        self.assertEqual(multisig[0]['self-publickey'], Alice.get_publickey(Alice.mainaddress()))
        self.assertEqual(multisig[0]['description'], description, "decription not match!")

        self.assertEqual(multisig[0]['m'], '2')
        self.assertEqual(multisig[0]['n'], '3')





