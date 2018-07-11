'''
test registerdid/modifydid when fork occured
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

            self.assertIn({'status':"registered",
            'address':role.mainaddress(),
            'symbol':role.did_symbol}, message)

    def test_1_fork_at_registerdid(self):
        self.make_partion()
        try:
            target_address = Alice.addresslist[-1]
            # registerdid and mine
            did_symbol = "YouShouldNotSeeThis.Avatar"
            Alice.send_etp(target_address, 10 ** 8)
            Alice.mining()

            ec, message = Alice.register_did(target_address, did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status': "registered",
                           'address': target_address,
                           'symbol': did_symbol}, message)
        finally:
            self.fork()

        ec, message = mvs_rpc.list_dids(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)
        self.assertNotIn({'status': "registered",
                       'address': target_address,
                       'symbol': Alice.did_symbol}, message)

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
            self.assertIn(asset_symbol, message)

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

    def test_3_fork_at_change_did(self):
        self.make_partion()
        try:
            target_addr = Cindy.addresslist[-1]
            Alice.send_etp(target_addr, 10**8)
            Alice.mining()

            ec, message = mvs_rpc.change_did(Cindy.name, Cindy.password, target_addr, Cindy.did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            expect = {
                u'status': u'registered',
                u'symbol': Cindy.did_symbol,
                u'address': target_addr
            }

            ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message)

            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message)


            target_addr = Cindy.addresslist[-2]
            Alice.send_etp(target_addr, 10**8)
            Alice.mining()

            ec, message = mvs_rpc.change_did(Cindy.name, Cindy.password, target_addr, Cindy.did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining()

            expect = {
                u'status': u'registered',
                u'symbol': Cindy.did_symbol,
                u'address': target_addr
            }

            ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message)


            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0, message)
            self.assertIn(expect, message)

        finally:
            self.fork()

        expect = {
            u'status': u'registered',
            u'symbol': Cindy.did_symbol,
            u'address': Cindy.mainaddress()
        }

        ec, message = mvs_rpc.list_dids(Cindy.name, Cindy.password)
        self.assertEqual(ec, 0, message)
        self.assertIn(expect, message)

        ec, message = mvs_rpc.list_dids()
        self.assertEqual(ec, 0, message)
        self.assertIn(expect, message)

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



    def test_5_fork_at_register(self):
        self.remote_ip=10.10.10.37
        
        did_symbol = "test_fork_registerdiid"+common.get_random_str()
        rmtName = Zac.name+common.get_random_str()
        Zac.new_address(2)
        mvs.remote_call(self.remote_ip, mvs_rpc.import_account)(rmtName, "123", Zac.mnemonic,2)
        Alice.sendmore_etp(Zac.addresslist[0]+":100000000" , Zac.addresslist[1]+":100000000")
        Alice.mining()

        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        pre_height = message[0]
        print "pre_height:"+fork_height

        self.make_partion()
        try:
            # fork
            Alice.mining()
            ec, message = Zac.register_did(Zac.addresslist[0], did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining(10)

            ec, message = mvs_rpc.get_info()
            self.assertEqual(ec, 0, message)
            fork_height = message[0]

            while  fork_height < pre_height+11:
                time.sleep(1)
                self.assertEqual(ec, 0, message)
                fork_height = message[0]
                print "fork_height:"+fork_height
              
        finally:
            # main chain
            Alice.mining(2)
            ec, message = mvs.remote_call(self.remote_ip,mvs_rpc.register_did)(rmtName, "123", Zac.addresslist[1],did_symbol)
            self.assertEqual(ec, 0, message)
            Alice.mining(20)
            
            ec, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.get_info)()
            self.assertEqual(ec, 0, message)
            main_height = message[0]

            while  fork_height < pre_height+22:
                time.sleep(1)
                ec, message = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.get_info)()
                self.assertEqual(ec, 0, message)
                main_height = message[0]
                print "main_height:"+main_height


        ec, message = mvs_rpc.add_node( self.remote_ip+':5251')
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.list_didaddresses(did_symbol)
        self.assertEqual(ec, 0, message)
        self.assertEqual(message[0]["address"], Zac.addresslist[1], message)
