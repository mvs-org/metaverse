import time
from utils import common
from TestCase.MVSTestCase import *

class TestCert(MVSTestCaseBase):
    need_mine = False

    def test_0_issuecert(self):
        Alice.ensure_balance()

        '''
        Alice create asset and cert
        '''
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        exist_asset = asset_symbol
        exist_domain_cert = domain_symbol

        test_cert_symbol = domain_symbol + ".CERT.TO.BOB"
        invalid_naming_cert = common.get_random_str()


        #account password error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password + '1', Bob.did_symbol, test_cert_symbol, 'naming')
        self.assertEqual(ec, 1000, message)
        Alice.mining()

        #symbol check
        # 1 -- length
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "X"*65, 'naming')
        self.assertEqual(ec, 5011, message)

        # 2 -- invalid char
        spec_char_lst = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<>/?"
        for char in spec_char_lst:
            ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, test_cert_symbol + char, 'naming')
            self.assertEqual(ec, 1000, message)
            self.assertEqual(message, "symbol must be alpha or number or dot", message)

        # check cert symbol -- invalid format
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, invalid_naming_cert, 'naming',
                                            fee=None)
        self.assertEqual(ec, 5012, message)

        # did not exist
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Zac.did_symbol, test_cert_symbol, 'naming')
        self.assertEqual(ec, 7006, message)

        # cert type error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, test_cert_symbol, "naming1")
        self.assertEqual(ec, 5017, message)

        # no domain cert owned
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, invalid_naming_cert + ".2BOB2", 'naming')
        self.assertEqual(ec, 5019, message)

        # issue cert success
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, test_cert_symbol, "naming")
        self.assertEqual(ec, 0, message)
        Alice.mining()

        # cert already exist error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, test_cert_symbol, "naming")
        self.assertEqual(ec, 5018, message)


    def test_1_issuecert_success(self):
        '''
        Alice create asset and cert
        '''
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        '''
        Alice issue cert to Bob.
        '''
        cert_symbol = Alice.issue_naming_cert(Bob, domain_symbol)
        Alice.mining()

        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password, cert_symbol, True)
        self.assertEqual(ec, 0, message)

        expect = {u'owner': Bob.did_symbol,
                  u'symbol': cert_symbol,
                  u'cert': u'naming',
                  u'address': Bob.mainaddress()}
        self.assertEqual(len(message['assetcerts']), 1, message)
        self.assertEqual(expect, message['assetcerts'][0])


    def test_2_transfercert(self):
        '''
        Alice create asset and cert
        '''
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        exist_asset = asset_symbol
        exist_domain_cert = domain_symbol

        naming_cert_symbol = Alice.issue_naming_cert(Bob, domain_symbol)
        Alice.mining()
        '''
        '''

        domain_cert_symbol = domain_symbol
        not_issued_symbol = domain_cert_symbol+'.2ND'+common.get_random_str()

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
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, naming_cert_symbol,
                                            'naming',
                                            fee=None)
        self.assertEqual(ec, 5020, message)

        # check fee
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, domain_cert_symbol,
                                            'domain',
                                            fee=0)
        self.assertEqual(ec, 5005, message)


    def test_3_transfercert_success(self):
        '''
        Alice create asset and cert
        '''
        time.sleep(1);
        domain_symbol, asset_symbol = Alice.create_random_asset()
        Alice.mining()

        naming_cert_symbol = Alice.issue_naming_cert(Bob, domain_symbol)
        Alice.mining()

        Alice.send_etp(Bob.mainaddress(), 20 * 10 ** 8)
        Alice.mining()
        '''
        '''

        ec, message = mvs_rpc.transfer_cert(Bob.name, Bob.password, Cindy.did_symbol, naming_cert_symbol, 'naming')
        self.assertEqual(ec, 0, message)

        Alice.mining()

        ec, message = mvs_rpc.get_accountasset(Cindy.name, Cindy.password, naming_cert_symbol, True)
        self.assertEqual(ec, 0, message)

        expect = {u'owner': Cindy.did_symbol,
                  u'symbol': naming_cert_symbol,
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
        # not allowed to secondary_issue
        domain_symbol, asset_symbol = Alice.create_random_asset(secondary=0)
        Alice.mining()

        # asset not allowed to secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 5015, message)

        # issue the asset
        # asset can be secondary issue freely
        time.sleep(1);
        domain_symbol, asset_symbol = Alice.create_random_asset(secondary=-1)
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


    def test_5_secondaryissue_success(self):
        time.sleep(1);
        domain_symbol, asset_symbol = Alice.create_random_asset(secondary=-1)  # asset can be secondary issue freely
        Alice.mining()

        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol)
        self.assertEqual(ec, 0, message)
        Alice.mining()


    def test_6_issue_with_attenuation_model(self):
        #
        # attenuation_model type 1
        #
        time.sleep(1);
        domain_symbol, asset_symbol = Alice.create_random_asset(is_issue=False, secondary=-1)
        Alice.mining()

        # invalid model type
        model_type = "invalid"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # invalid LQ
        model_type = "TYPE=1;LQ=0;LP=6001;UN=3"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # invalid LP
        model_type = "TYPE=1;LQ=9001;LP=0;UN=3"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # invalid UN
        model_type = "TYPE=1;LQ=9001;LP=6001;UN=0"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)
        Alice.mining()

        # invalid model type
        model_type = "TYPE=3;LQ=9001;LP=6001;UN=3"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # success
        model_type = "TYPE=1;LQ=9001;LP=6001;UN=3"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #
        # attenuation_model type 2
        #
        domain_symbol, asset_symbol = Alice.create_random_asset(is_issue=False, secondary=-1)
        Alice.mining()

        # UC size dismatch UQ size
        model_type = "TYPE=2;LQ=9001;LP=6001;UN=3;UC=2000,2000;UQ=3000,3000,3001"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        model_type = "TYPE=2;LQ=9001;LP=6001;UN=3;UC=2000,2000,2001;UQ=3000,3000,3001"
        ec, message = Alice.issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 0, message)
        Alice.mining()


    def test_7_secondary_issue_with_attenuation_model(self):
        # create asset
        time.sleep(1);
        domain_symbol, asset_symbol = Alice.create_random_asset(is_issue=True, secondary=-1)
        Alice.mining()

        #
        # attenuation_model type 1
        #

        # invalid model type
        model_type = "invalid"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # invalid LQ
        model_type = "TYPE=1;LQ=0;LP=6001;UN=3"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # invalid LP
        model_type = "TYPE=1;LQ=9001;LP=0;UN=3"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # invalid UN
        model_type = "TYPE=1;LQ=9001;LP=6001;UN=0"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)
        Alice.mining()

        # invalid model type
        model_type = "TYPE=3;LQ=9001;LP=6001;UN=3"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        # success
        model_type = "TYPE=1;LQ=9001;LP=6001;UN=3"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        #
        # attenuation_model type 2
        #

        # UC size dismatch UQ size
        model_type = "TYPE=2;LQ=9001;LP=6001;UN=3;UC=2000,2000;UQ=3000,3000,3001"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 5016, message)

        model_type = "TYPE=2;LQ=9001;LP=6001;UN=3;UC=2000,2000,2001;UQ=3000,3000,3001"
        ec, message = Alice.secondary_issue_asset_with_symbol(asset_symbol, model_type)
        self.assertEqual(ec, 0, message)
        Alice.mining()
