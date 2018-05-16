from utils import common
from TestCase.MVSTestCase import *

class TestCert(MVSTestCaseBase):
    need_mine = False
    def test_0_issuecert(self):
        #account password error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password + '1', Bob.did_symbol, "ALICE.TO_BOB", "NAMING")
        self.assertEqual(ec, 1000, message)

        #symbol check
        # 1 -- length
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "X"*65, "NAMING")
        self.assertEqual(ec, 5011, message)

        # 2 -- invalid char
        spec_char_lst = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<>/?"
        for char in spec_char_lst:
            ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.TO.BOB" + char, "NAMING")
            self.assertEqual(ec, 1000, message)
            self.assertEqual(message, "symbol must be alpha or number or dot", message)

        # did not exist
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Zac.did_symbol, "ALICE.2BOB", "NAMING")
        self.assertEqual(ec, 7006, message)

        #cert type error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.2BOB", "NAMING1")
        self.assertEqual(ec, 5017, message)

        #fee error
        ec, message = mvs_rpc.issue_cert(Alice.name, Alice.password, Bob.did_symbol, "ALICE.2BOB", "NAMING", 0)
        self.assertEqual(ec, 5005, message)

    def test_1_issuecert_success(self):
        '''
        Alice issue cert to Bob.
        '''
        cert_symbol = Alice.issue_cert(Bob)
        Alice.mining()
        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password, cert_symbol, True)
        self.assertEqual(ec, 0, message)

        expect = {u'owner': Bob.did_symbol,
                  u'symbol': cert_symbol,
                  u'cert': u"NAMING",
                  u'address': Bob.mainaddress()}
        self.assertEqual(len(message['assetcerts']), 1, message)
        self.assertEqual(expect, message['assetcerts'][0])

    def test_2_transfercert(self):
        asset_symbol_lst = Alice.asset_symbol.split('.')
        domain_cert_symbol = asset_symbol_lst[0]
        second_cert_symbol = domain_cert_symbol+'.2ND'+common.get_timestamp()

        # account password match error
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password+'1', Bob.did_symbol, domain_cert_symbol, "NAMING",
                                            fee=None)
        self.assertEqual(ec, 1000, message)

        # did_symbol not exist
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Zac.did_symbol, domain_cert_symbol, "NAMING",
                                            fee=None)
        self.assertEqual(ec, 7006, message)

        # check cert symbol -- not issued
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, second_cert_symbol, "NAMING",
                                            fee=None)
        self.assertEqual(ec, 5017, message)

        # check cert symbol -- length error
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, "X"*65,
                                            "NAMING",
                                            fee=None)
        self.assertEqual(ec, 5011, message)

        # check cert symbol -- owned by some other
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, "BOB",
                                            "NAMING",
                                            fee=None)
        self.assertEqual(ec, 5017, message)

        # check fee
        ec, message = mvs_rpc.transfer_cert(Alice.name, Alice.password, Bob.did_symbol, domain_cert_symbol,
                                            "NAMING",
                                            fee=0)
        self.assertEqual(ec, 5005, message)

    def test_3_transfercert_success(self):
        "Alice -> Bob -> Cindy: ALICE.2BOB.timestamp"
        cert_symbol = Alice.issue_cert(Bob)
        Alice.mining()
        ec, message = mvs_rpc.transfer_cert(Bob.name, Bob.password, Cindy.did_symbol, cert_symbol, "NAMING")
        self.assertEqual(ec,0,message)

        Alice.mining()

        ec, message = mvs_rpc.get_accountasset(Cindy.name, Cindy.password, cert_symbol, True)
        self.assertEqual(ec, 0, message)

        expect = {u'owner': Cindy.did_symbol,
                  u'symbol': cert_symbol,
                  u'cert': u'NAMING',
                  u'address': Cindy.mainaddress()}
        self.assertEqual(len(message['assetcerts']), 1, message)
        self.assertEqual(expect, message['assetcerts'][0])

    def test_4_secondaryissue(self):
        # account password match error
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password+"1", Alice.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 1000, message)

        # did_symbol not exist
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Zac.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 7006, message)

        # did_symbol belong to some other
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Bob.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 4003, message)

        # asset is not issued
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 5010, message)

        # issue the asset
        Alice.create_asset(secondary=0) # not allowed to secondary_issue
        Alice.mining()

        # asset not allowed to secondary_issue
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol,
                                              volume=100, model=None, fee=None)
        self.assertEqual(ec, 5015, message)

        # issue the asset
        Alice.create_asset(secondary=-1)  # asset can be secondary issue freely
        Alice.mining()

        # asset belong to some other
        ec, message = mvs_rpc.secondary_issue(Bob.name, Bob.password, Bob.did_symbol, Alice.asset_symbol, volume=100, model=None, fee=None)
        self.assertEqual(ec, 5017, message)

        # asset volume 0
        #ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol, volume=0, model=None, fee=None)
        #self.assertEqual(ec, 5010, message)

        # asset model -- TODO

        # fee 0
        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol, volume=100, model=None, fee=0)
        self.assertEqual(ec, 5005, message)

    def test_5_secondaryissue_success(self):
        Alice.create_asset(secondary=-1)  # asset can be secondary issue freely
        Alice.mining()

        ec, message = mvs_rpc.secondary_issue(Alice.name, Alice.password, Alice.did_symbol, Alice.asset_symbol, volume=100, model=None, fee=None)
        self.assertEqual(ec, 0, message)

        Alice.mining()