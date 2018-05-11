import random
from TestCase.MVSTestCase import *

class TestAccount(MVSTestCaseBase):
    roles = (Alice,)
    need_mine = False
    def test_0_new_account(self):
        #account already exist
        ec, message = mvs_rpc.new_account(Alice.name, "1")
        self.assertEqual(ec, 3001, message)

    def test_1_get_account(self):
        #password error
        ec, message = mvs_rpc.get_account(Alice.name, Alice.password+'1', Alice.lastword())
        self.assertEqual(ec, 1000, message)

        #lastword error
        ec, message = mvs_rpc.get_account(Alice.name, Alice.password, Alice.mnemonic[-2])
        self.assertEqual(ec, 1000, message)

        ec, message = mvs_rpc.get_account(Alice.name, Alice.password, Alice.lastword())
        self.assertEqual(ec, 0, message)
        mnemonic, address_num = message

        self.assertEqual(mnemonic, ' '.join(Alice.mnemonic))
        self.assertEqual(address_num, 10)

    def test_2_delete_account(self):
        # password error
        ec, message = mvs_rpc.delete_account(Alice.name, Alice.password + '1', Alice.lastword())
        self.assertEqual(ec, 1000, message)

        # lastword error
        ec, message = mvs_rpc.delete_account(Alice.name, Alice.password, Alice.mnemonic[-2])
        self.assertEqual(ec, 1000, message)

        ec, message = mvs_rpc.delete_account(Alice.name, Alice.password, Alice.lastword())
        self.assertEqual(ec, 0, message)

        #TODO: this shall access the database directly~~
        #check account-address

        # check account-asset

        Alice.create()

    def test_3_import_account(self):
        # account already exist
        ec, message = mvs_rpc.import_account(Alice.name, Alice.password, 'Alice.mnemonic')
        self.assertEqual(ec, 3001, message)


        Alice.delete()
        # invalid mnemonic word at random position
        i = random.randint(0, len(Alice.mnemonic) - 1)
        mnemonic = Alice.mnemonic[:i] + ["@invalid"] +  Alice.mnemonic[i+1:]

        ec, message = mvs_rpc.import_account(Alice.name, Alice.password, ' '.join(mnemonic))
        self.assertEqual(ec, 9202, message)

        #create 3 address
        ec, message = mvs_rpc.import_account(Alice.name, Alice.password, ' '.join(Alice.mnemonic), 3)
        self.assertEqual(ec, 0, message)

        ec, addresses = mvs_rpc.list_addresses(Alice.name, Alice.password)
        self.assertEqual(ec, 0, addresses)
        addresses.reverse()
        self.assertEqual(addresses, Alice.addresslist[:3])

    def test_4_changepasswd(self):
        # password error
        ec, message = mvs_rpc.change_passwd(Alice.name, Alice.password+'1', 'Alice.mnemonic')
        self.assertEqual(ec, 1000, message)

        new_password = 'test_4_changepasswd' # -> the test case name
        old_password = Alice.password

        ec, message = mvs_rpc.change_passwd(Alice.name, Alice.password, new_password)
        self.assertEqual(ec, 0, message)
        Alice.password = new_password  # -> so that the account can be deleted when tearDown

        ec, addresses = mvs_rpc.list_addresses(Alice.name, new_password)
        self.assertEqual(ec, 0, addresses)
        self.assertEqual(addresses, Alice.addresslist)

        #try the old password
        ec, message = mvs_rpc.list_addresses(Alice.name, old_password)
        self.assertEqual(ec, 1000, message)

        Alice.delete()
        Alice.password = old_password
        Alice.create()






