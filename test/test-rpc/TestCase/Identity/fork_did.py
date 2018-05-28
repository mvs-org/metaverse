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
            target_address = Alice.addresslist[-1]
            # issuedid and mine
            did_symbol = "YouShouldNotSeeThis.Avatar"
            Alice.send_etp(target_address, 10 ** 8)
            Alice.mining()

            ec, message = Alice.issue_did(target_address, did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status': "issued",
                           'address': target_address,
                           'symbol': did_symbol}, message['dids'])
        finally:
            self.fork()

        ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertNotIn({'status': "issued",
                       'address': target_address,
                       'symbol': Alice.did_symbol}, message['dids'])

    def test_2_fork_at_issueasset(self):
        self.make_partion()

        asset_symbol = None
        domain_symbol = None
        try:
            domain = (u'Not1Exist' + common.get_random_str()).upper()
            domain_symbol, asset_symbol = Alice.create_random_asset(domain_symbol=domain)
            Alice.mining()

            # check asset
            ec, message = mvs_rpc.get_asset( )
            self.assertEqual(ec, 0, message)
            self.assertIn(asset_symbol, message["assets"])

            addressassets = Alice.get_addressasset(Alice.mainaddress())
            addressasset = filter(lambda a: a.symbol == asset_symbol, addressassets)
            self.assertEqual(len(addressasset), 1)

            # check domain cert
            certs = Alice.get_addressasset(Alice.mainaddress(), True)
            cert = filter(lambda a: a.symbol == domain_symbol, certs)
            self.assertEqual(len(cert), 1)

        finally:
            self.fork()

        # check asset
        ec, message = mvs_rpc.get_asset( )
        self.assertEqual(ec, 0, message)
        self.assertNotIn(asset_symbol, message["assets"])

        addressassets = Alice.get_addressasset(Alice.mainaddress())
        addressasset = filter(lambda a: a.symbol == asset_symbol, addressassets)
        self.assertEqual(len(addressasset), 0)

        # check domain cert
        certs = Alice.get_addressasset(Alice.mainaddress(), True)
        cert = filter(lambda a: a.symbol == domain_symbol, certs)
        self.assertEqual(len(cert), 0)

    def test_3_fork_at_modify_did(self):
        self.make_partion()
        try:
            target_addr = Cindy.addresslist[-1]
            Alice.send_etp(target_addr, 10**8)
            Alice.mining()

            ec, message = mvs_rpc.modify_did(Cindy.name, Cindy.password, target_addr, Cindy.did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            expect = {
                u'status': u'issued',
                u'symbol': Cindy.did_symbol,
                u'address': target_addr
            }

            ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])

            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])


            target_addr = Cindy.addresslist[-2]
            Alice.send_etp(target_addr, 10**8)
            Alice.mining()

            ec, message = mvs_rpc.modify_did(Cindy.name, Cindy.password, target_addr, Cindy.did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            expect = {
                u'status': u'issued',
                u'symbol': Cindy.did_symbol,
                u'address': target_addr
            }

            ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])


            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message['dids'])

        finally:
            self.fork()

        expect = {
            u'status': u'issued',
            u'symbol': Cindy.did_symbol,
            u'address': Cindy.mainaddress()
        }

        ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
        self.assertEqual(ec, 0, message)
        self.assertIn(expect, message['dids'])

        ec, message = mvs_rpc.list_dids()
        self.assertEqual(ec, 0, message)
        self.assertIn(expect, message['dids'])

    def test_4_fork_at_issuecert(self):
        self.make_partion()

        cert_symbol = None
        try:
            domain = (u'Not2Exist' + common.get_random_str()).upper()
            domain_symbol, asset_symbol = Alice.create_random_asset(domain_symbol=domain)
            Alice.mining()

            cert_symbol = (domain_symbol + ".NAMING").upper();
            ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Alice.did_symbol, cert_symbol, "NAMING")
            self.assertEqual(ec, 0, message)
            Alice.mining()

            # check naming cert
            certs = Alice.get_addressasset(Alice.didaddress(), True)


            cert = filter(lambda a: a.symbol == cert_symbol, certs)
            self.assertEqual(len(cert), 1)

        finally:
            self.fork()

        # check cert
        certs = Alice.get_addressasset(Alice.didaddress(), True)
        cert = filter(lambda a: a.symbol == cert_symbol, certs)
        self.assertEqual(len(cert), 0)
