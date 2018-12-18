from TestCase.MVSTestCase import *

hex_script= "63522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b3752102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68"
p2sh_address = "3443Q4tSg9s11kwYKdn1eCMx3H8UcZdgPE"

class TestAddress(MVSTestCaseBase):
    need_mine = False
    def test_0_import_adress(self):
        ec, message = mvs_rpc.import_address(Alice.name, "xxx", hex_script)
        self.assertEqual(ec, 1000, message)

        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, "xxx")
        self.assertEqual(ec, 5502, message)

        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, Zac.mainaddress())
        self.assertEqual(ec, 5502, message)

    def test_1_import_adress(self):
        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, p2sh_address)
        self.assertEqual(ec, 0, message)

    def test_2_import_adress(self):
        description = "test_2_import_adress"
        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, hex_script, description)
        self.assertEqual(ec, 0, message)
        self.assertEqual(message['address'], p2sh_address)
        self.assertEqual(message['script'], 'if 2 [ 02578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573 ] [ 0380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d11 ] 2 checkmultisig else [ 64 ] checksequenceverify drop [ 02578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573 ] checksig endif')


        ec, message = mvs_rpc.dump_keyfile(Alice.name, Alice.password, Alice.lastword(), keyfile="any", to_report=True)
        self.assertEqual(ec, 0, message)

        self.assertEqual(message['scripts'][0]['address'], p2sh_address)
        self.assertEqual(message['scripts'][0]['script'], hex_script)
        self.assertEqual(message['scripts'][0]['description'], description)

    def test_3_multisig_and_script(self):
        description = "test_3_multisig_and_script"
        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, hex_script, description)
        self.assertEqual(ec, 0, message)
        Alice.new_multisigaddress(description, [Bob, Cindy], 2)

        ec, message = mvs_rpc.dump_keyfile(Alice.name, Alice.password, Alice.lastword(), keyfile="any", to_report=True)
        self.assertEqual(ec, 0, message)

        self.assertEqual(len( message['scripts'] ), 1)
        self.assertEqual(len(message['multisigs']), 1)

    def test_4_multisig_and_script(self):
        '''
        change the order in test_3_multisig_and_script
        :return:
        '''
        description = "test_4_multisig_and_script"

        Alice.new_multisigaddress(description, [Bob, Cindy], 2)

        ec, message = mvs_rpc.import_address(Alice.name, Alice.password, hex_script, description)
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.dump_keyfile(Alice.name, Alice.password, Alice.lastword(), keyfile="any", to_report=True)
        self.assertEqual(ec, 0, message)

        self.assertEqual(len( message['scripts'] ), 1)
        self.assertEqual(len(message['multisigs']), 1)

