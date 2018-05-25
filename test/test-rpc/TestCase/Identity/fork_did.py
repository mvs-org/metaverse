'''
test issuedid/modifydid when fork occured
'''
from utils import common
from TestCase.MVSTestCase import *

class TestFork(ForkTestCase):
    def test_0_fork_at_send(self):
        self.make_partion()
        # make transaction and mine
        Alice.send_etp(Bob.mainaddress(), 10**8)
        Alice.mining()

        self.fork()

        # check if Alice & Bob's address_did record is cleaned
        for role in [Alice, Bob]:
            ec, message = mvs_rpc.list_dids(role.name, role.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status':"issued",
            'address':role.mainaddress(),
            'symbol':role.did_symbol}, message['dids'])

    def test_1_fork_at_issuedid(self):
        self.make_partion()
        try:
            # issuedid and mine
            did_symbol = "YouShouldNotSeeThis2.Avatar"
            Alice.send_etp(Alice.addresslist[-2], 10 ** 8)
            Alice.mining()

            ec, message = Alice.issue_did(Alice.addresslist[-2], did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status': "issued",
                           'address': Alice.addresslist[-2],
                           'symbol': did_symbol}, message['dids'])
        finally:
            self.fork()

        self.assertNotIn({'status': "issued",
                       'address': Alice.addresslist[-2],
                       'symbol': Alice.did_symbol}, message['dids'])

    def test_2_fork_at_issueasset(self):
        self.make_partion()

        Alice.create_asset()
        Alice.mining()

        ec, message = mvs_rpc.get_asset( )
        self.assertEqual(ec, 0, message)
        self.assertIn(Alice.asset_symbol, message["assets"])


        addressassets = Alice.get_addressasset(Alice.mainaddress())
        addressasset = filter(lambda a: a.symbol == Alice.asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 1)

        self.fork()

        ec, message = mvs_rpc.get_asset( )
        self.assertEqual(ec, 0, message)
        self.assertNotIn(Alice.asset_symbol, message["assets"])

        addressassets = Alice.get_addressasset(Alice.mainaddress())
        addressasset = filter(lambda a: a.symbol == Alice.asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 0)
