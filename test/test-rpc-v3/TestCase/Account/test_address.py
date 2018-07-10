import random
from TestCase.MVSTestCase import *

class TestAccount(MVSTestCaseBase):
    roles = (Alice,)
    need_mine = False

    def test_0_new_address(self):
        #password error
        ec, message = mvs_rpc.new_address(Alice.name, Alice.password+'1')
        self.assertEqual(ec, 1000, message)

        #check address_count
        ec, message = mvs_rpc.new_address(Alice.name, Alice.password, 0)
        self.assertEqual(ec, 4004, message)

        ec, message = mvs_rpc.new_address(Alice.name, Alice.password, 0x00100000)
        self.assertEqual(ec, 4004, message)

        ec, message = mvs_rpc.new_address(Alice.name, Alice.password, 11)
        self.assertEqual(ec, 0, message)

    def test_1_list_addresses(self):
        # password error
        ec, message = mvs_rpc.list_addresses(Alice.name, Alice.password + '1')
        self.assertEqual(ec, 1000, message)

        ec, addresses = mvs_rpc.list_addresses(Alice.name, Alice.password)
        self.assertEqual(ec, 0, addresses)
        addresses.sort()
        alice_addresses = Alice.addresslist[:]
        alice_addresses.sort()
        self.assertEqual(addresses, alice_addresses)

    def test_2_check_address(self):
        for address in Alice.addresslist:
            ec, message = mvs_rpc.check_address(address)
            self.assertEqual(ec, 0, message)