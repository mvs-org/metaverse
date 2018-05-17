from utils import common
from TestCase.MVSTestCase import *

class TestCert(MVSTestCaseBase):
    need_mine = False


    def test_0_issuecert(self):
        invalid_naming_cert = common.get_timestamp()

        #account password error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password + '1', Bob.did_symbol, "ALICE.TO_BOB", 'naming')
        self.assertEqual(ec, 1000, message)
        Alice.mining()

        #symbol check
        # 1 -- length
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "X"*65, 'naming')
        self.assertEqual(ec, 5011, message)

        # 2 -- invalid char
        spec_char_lst = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<>/?"
        for char in spec_char_lst:
            ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.TO.BOB" + char, 'naming')
            self.assertEqual(ec, 1000, message)
            self.assertEqual(message, "symbol must be alpha or number or dot", message)

        # check cert symbol -- invalid format
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, invalid_naming_cert, 'naming',
                                            fee=None)
        self.assertEqual(ec, 5012, message)

        # did not exist
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Zac.did_symbol, "ALICE.2BOB", 'naming')
        self.assertEqual(ec, 7006, message)

        # cert type error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.2BOB", "naming1")
        self.assertEqual(ec, 5017, message)

        # cert already exist error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.2BOB", "naming1")
        self.assertEqual(ec, 5017, message)

        # no domain cert owned
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.2BOB2", 'naming')
        self.assertEqual(ec, 5019, message)


    def test_1_issuecert_success(self):
        '''
        Alice create asset and cert
        '''
        domain_symbol = u"ALICE" + common.get_timestamp()
        asset_symbol = domain_symbol + ".ASSET"
        Alice.create_asset_with_symbol(asset_symbol)
        Alice.mining(10)

        exist_asset = asset_symbol
        exist_domain_cert = domain_symbol

        '''
        Alice issue cert to Bob.
        '''
        cert_symbol = Alice.issue_naming_cert(Bob, exist_domain_cert)
        Alice.mining()

        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password, cert_symbol, True)
        self.assertEqual(ec, 0, message)

        expect = {u'owner': Bob.did_symbol,
                  u'symbol': cert_symbol,
                  u'cert': u'naming',
                  u'address': Bob.mainaddress()}
        self.assertEqual(len(message['assetcerts']), 1, message)
        self.assertEqual(expect, message['assetcerts'][0])

        exist_naming_cert = cert_symbol


    def test_2_transfercert(self):
        '''
        Alice create asset and cert
        '''
        domain_symbol = u"ALICE" + common.get_timestamp()
        asset_symbol = domain_symbol + ".ASSET"
        Alice.create_asset_with_symbol(asset_symbol)
        Alice.mining(100)

        exist_asset = asset_symbol
        exist_domain_cert = domain_symbol

        cert_symbol = Alice.issue_naming_cert(Bob, exist_domain_cert)
        Alice.mining()

        exist_naming_cert = cert_symbol
        '''
        '''

        domain_cert_symbol = exist_domain_cert
        not_issued_symbol = domain_cert_symbol+'.2ND'+common.get_timestamp()

        # account password match error
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password+'1', Bob.did_symbol, domain_cert_symbol, 'naming',
                                            fee=None)
        self.assertEqual(ec, 1000, message)

        # did_symbol not exist
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, "InvalidDID", domain_cert_symbol, 'naming',
                                            fee=None)
        self.assertEqual(ec, 7006, message)

        # check cert symbol -- length error
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, "X"*65,
                                            'naming',
                                            fee=None)
        self.assertEqual(ec, 5011, message)

        # check cert symbol -- not issued
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, not_issued_symbol, 'naming',
                                            fee=None)
        self.assertEqual(ec, 5019, message)

        # check cert symbol -- owned by some other
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, exist_naming_cert,
                                            'naming',
                                            fee=None)
        self.assertEqual(ec, 5020, message)

        # check fee
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, exist_domain_cert,
                                            'domain',
                                            fee=0)
        self.assertEqual(ec, 5005, message)


    def test_3_transfercert_success(self):
        '''
        Alice create asset and cert
        '''
        domain_symbol = u"ALICE" + common.get_timestamp()
        asset_symbol = domain_symbol + ".ASSET"
        Alice.create_asset_with_symbol(asset_symbol)
        Alice.mining()

        exist_asset = asset_symbol
        exist_domain_cert = domain_symbol

        cert_symbol = Alice.issue_naming_cert(Bob, exist_domain_cert)
        Alice.mining()

        exist_naming_cert = cert_symbol

        Bob.mining(1200)
        '''
        '''

        ec, message = mvs_rpc.transfer_cert(Bob.name, Bob.password, Cindy.did_symbol, cert_symbol, 'naming')
        self.assertEqual(ec, 0, message)

        Alice.mining()

        ec, message = mvs_rpc.get_accountasset(Cindy.name, Cindy.password, cert_symbol, True)
        self.assertEqual(ec, 0, message)

        expect = {u'owner': Cindy.did_symbol,
                  u'symbol': cert_symbol,
                  u'cert': u'naming',
                  u'address': Cindy.mainaddress()}
        self.assertEqual(len(message['assetcerts']), 1, message)
        self.assertEqual(expect, message['assetcerts'][0])


    def test_4_secondaryissue(self):
        # account password match error
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password+"1", Alice.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 1000, message)
        Alice.mining()

        # did_symbol not exist
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Zac.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 7006, message)
        Alice.mining()

        # did_symbol belong to some other
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Bob.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 4003, message)
        Alice.mining()

        # asset is not issued
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 5010, message)
        Alice.mining()

        # issue the asset
        Alice.create_asset(secondary=0) # not allowed to secondary_issue
        Alice.mining()

        # asset not allowed to secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 5015, message)

        # issue the asset
        domain_symbol = u"ALICE" + common.get_timestamp()
        asset_symbol = domain_symbol + ".ASSET"

        # asset can be secondary issue freely
        Alice.create_asset_with_symbol(symbol=asset_symbol, secondary=-1)
        Alice.mining()

        # asset belong to some other
        ec, message = mvs_rpc.secondary_issue(Bob.name, Bob.password, Bob.did_symbol, asset_symbol, volume=100, model=None, fee=None)
        self.assertEqual(ec, 5020, message)

        # asset volume 0
        #ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, asset_symbol, volume=0, model=None, fee=None)
        #self.assertEqual(ec, 5010, message)

        # fee 0
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, asset_symbol, volume=100, model=None, fee=0)
        self.assertEqual(ec, 5005, message)

        # asset model -- TODO

    def test_5_secondaryissue_success(self):
        Alice.create_asset(secondary=-1)  # asset can be secondary issue freely
        Alice.mining()

        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol, volume=100, model=None, fee=None)
        self.assertEqual(ec, 0, message)

        Alice.mining()