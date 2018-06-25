#!/usr/bin/python
# -*- coding: utf-8 -*-
import random
from TestCase.MVSTestCase import *

class TestAssetBoundary(MVSTestCaseBase):
    '''
    create asset depends on did.
    so Alice-Frank shall create their dids before running this test case
    '''
    need_mine = False

    def getExistAssetSymbol(self):
        # symbol is already used.
        ec, message = mvs_rpc.get_asset()
        self.assertEqual(ec, 0, message)

        exist_assets = message["assets"]
        if not exist_assets:
            return None
        # pickup existing asset symbol by random
        i = random.randint(0, len(exist_assets) - 1)
        return exist_assets[i]

    def test_0_check_asset_symbol(self):
        spec_char_lst = "`~!@#$%^&*()-_=+[{]}\\|;:'\",<>/?"
        for char in spec_char_lst:
            ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, Alice.asset_symbol + char, 100, Alice.did_symbol)
            self.assertEqual(ec, 1000, message)
            self.assertEqual(message, "symbol must be alpha or number or dot", message)

    def test_1_create_asset(self):
        #account password match error
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password + '1', Alice.asset_symbol, 100, Alice.did_symbol)
        self.assertEqual(ec, 1000, message)

        #aasset symbol can not be empty.
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "", 100, Alice.did_symbol)
        self.assertEqual(ec, 5011, message)

        #asset symbol length must be less than 64.
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 65, 100, Alice.did_symbol)
        self.assertEqual(ec, 5011, message)

        #asset description length must be less than 64.
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 64, 100, Alice.did_symbol, "x"*65)
        self.assertEqual(ec, 5007, message)

        #secondaryissue threshold value error, is must be -1 or in range of 0 to 100.
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 64, 100, Alice.did_symbol, "x" * 64, rate=-2)
        self.assertEqual(ec, 5015, message)

        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 64, 100, Alice.did_symbol, "x" * 64, rate=101)
        self.assertEqual(ec, 5015, message)

        #asset decimal number must less than 20.
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 64, 100, Alice.did_symbol, "x" * 64, rate=0, decimalnumber=-1)
        self.assertEqual(ec, 5002, message)
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 64, 100, Alice.did_symbol, "x" * 64, rate=0, decimalnumber=20)
        self.assertEqual(ec, 5002, message)

        #volume must not be zero.
        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "x" * 64, 0, Alice.did_symbol, "x" * 64, rate=0, decimalnumber=19)
        self.assertEqual(ec, 2003, message)
        #contain sensitive words

        ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, "JZM.xxx", 10, Alice.did_symbol, "x" * 64, rate=0, decimalnumber=19)
        self.assertEqual(ec, 5012, message)

        exist_symbol = self.getExistAssetSymbol()
        if exist_symbol:
            ec, message = mvs_rpc.create_asset(Alice.name, Alice.password, exist_symbol, 10, Alice.did_symbol, "x" * 64, rate=0, decimalnumber=19)
            self.assertEqual(ec, 5009, message)

    def test_2_issue_asset(self):
        ten_etp = 12 * (10 ** 8)
        # account password match error
        ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password + '2', Zac.asset_symbol, 1)
        self.assertEqual(ec, 1000, message)

        #issue asset fee less than 10 etp
        ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password, Zac.asset_symbol, ten_etp - 1)
        self.assertEqual(ec, 5006, message)

        #asset symbol length must be less than 64
        ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password, "x"*65, ten_etp)
        self.assertEqual(ec, 5011, message)

        #asset symbol is already exist in blockchain
        exist_symbol = self.getExistAssetSymbol()
        if exist_symbol:
            ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password, exist_symbol, ten_etp)
            self.assertEqual(ec, 5009, message)

        #asset not in local database
        ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password, Zac.asset_symbol, ten_etp)
        self.assertEqual(ec, 5010, message)

        def isDomainExist(domain):
            ec, message = mvs_rpc.get_asset()
            self.assertEqual(ec, 0, message)

            exist_assets = message["assets"]
            if not exist_assets:
                return False
            for i in exist_assets:
                if i.startswith(domain):
                    return True

        domain = Alice.domain_symbol + "."
        if isDomainExist(domain):
            asset_symbol = domain + Bob.asset_symbol
            # domain cert not belong to current account
            ec, message = mvs_rpc.create_asset(Bob.name, Bob.password, asset_symbol, 100, Bob.did_symbol)
            self.assertEqual(ec, 0, message)

            ec, message = mvs_rpc.issue_asset(Bob.name, Bob.password, asset_symbol, ten_etp)
            self.assertEqual(ec, 5020, message)

    def test_3_list_assets(self):
        # account password match error
        ec, message = mvs_rpc.list_assets(Zac.name, Zac.password + '3')
        self.assertEqual(ec, 1000, message)

        ec, message = mvs_rpc.list_assets(Alice.name, Alice.password)
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.list_assets(cert=True)
        self.assertEqual(ec, 0, message)

    def test_4_get_asset(self):
        #Illegal asset symbol length.
        ec, message = mvs_rpc.get_asset("x"*65)
        self.assertEqual(ec, 5011, message)

        #asset not exist
        ec, message = mvs_rpc.get_asset("JZM.xxx")
        self.assertEqual(ec, 0, message)
        self.assertEqual(message['assets'], None, message)

        #get cert
        ec, message = mvs_rpc.get_asset(cert=True)
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.get_asset("JZM.xxx", cert=True)
        self.assertEqual(ec, 0, message)

    def test_5_deletelocalasset(self):
        # account password match error
        ec, message = mvs_rpc.delete_localasset(Zac.name, Zac.password + '4', Zac.asset_symbol)
        self.assertEqual(ec, 1000, message)

        # asset not exist
        ec, message = mvs_rpc.delete_localasset(Zac.name, Zac.password, Zac.asset_symbol)
        self.assertEqual(ec, 5003, message)

        # asset not belong to Zac
        Frank.create_asset(False)
        ec, message = mvs_rpc.delete_localasset(Zac.name, Zac.password, Frank.asset_symbol)
        self.assertEqual(ec, 5003, message)

        Bob.create_asset(False)
        ec, message = mvs_rpc.delete_localasset(Bob.name, Bob.password, Bob.asset_symbol)
        self.assertEqual(ec, 0, message)

    def test_6_getaccountasset(self):
        # account password match error
        ec, message = mvs_rpc.get_accountasset(Zac.name, Zac.password + '5', Zac.asset_symbol)
        self.assertEqual(ec, 1000, message)

        # no asset
        ec, message = mvs_rpc.get_accountasset(Zac.name, Zac.password, Zac.asset_symbol)
        self.assertEqual(ec, 0, message)
        self.assertEqual({u'assets': None}, message)

        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password)
        self.assertEqual(ec, 0, message)
        orign = 0
        if message[u'assets']:
            orign = len(message[u'assets'])

        # with +1 asset
        Bob.create_asset(False)
        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(len(message["assets"]), orign+1, message)

        # with +2 assets
        ec, message = mvs_rpc.create_asset(Bob.name, Bob.password, Bob.asset_symbol + '1', 100, Bob.did_symbol)
        self.assertEqual(ec, 0, message)

        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password)
        self.assertEqual(ec, 0, message)
        self.assertEqual(len(message["assets"]), orign+2, message)

        # asset_symbol sepecified
        ec, message = mvs_rpc.get_accountasset(Bob.name, Bob.password, Bob.asset_symbol)
        self.assertEqual(ec, 0, message)
        self.assertEqual(len(message["assets"]), 1, message)

        # --cert specified
        ec, message = mvs_rpc.get_accountasset(Zac.name, Zac.password, cert=True)
        self.assertEqual(ec, 0, message)
        self.assertEqual(message["assetcerts"], None, message)

    def test_7_getaddressasset(self):
        # invalid address
        ec, message = mvs_rpc.get_addressasset(Zac.mainaddress()+'1')
        self.assertEqual(ec, 4010, message)

        # no asset
        ec, message = mvs_rpc.get_addressasset(Zac.mainaddress())
        self.assertEqual(ec, 0, message)
        self.assertEqual({u'assets': None}, message)

        # --cert specified
        ec, message = mvs_rpc.get_addressasset(Zac.mainaddress(), cert=True)
        self.assertEqual(ec, 0, message)

    def test_8_sendasset(self):
        # account password match error
        ec, message = mvs_rpc.send_asset(Zac.name, Zac.password + '6', "", Zac.asset_symbol, 1)
        self.assertEqual(ec, 1000, message)

        # invalid to address parameter
        ec, message = mvs_rpc.send_asset(Zac.name, Zac.password, "111", Zac.asset_symbol, 1)
        self.assertEqual(ec, 4010, message)

        # invalid amount parameter
        ec, message = mvs_rpc.send_asset(Zac.name, Zac.password, Zac.mainaddress(), Zac.asset_symbol, 0)
        self.assertEqual(ec, 5002, message)

    def test_9_sendassetfrom(self):
        # account password match error
        ec, message = mvs_rpc.send_asset_from(Zac.name, Zac.password + '6', "", "", Zac.asset_symbol, 1)
        self.assertEqual(ec, 1000, message)

        # invalid from address
        ec, message = mvs_rpc.send_asset_from(Zac.name, Zac.password, "111", "222", Zac.asset_symbol, 1)
        self.assertEqual(ec, 4015, message)

        # invalid to address
        ec, message = mvs_rpc.send_asset_from(Zac.name, Zac.password, Zac.mainaddress(), "", Zac.asset_symbol, 1)
        self.assertEqual(ec, 4012, message)

        # invalid amount parameter
        ec, message = mvs_rpc.send_asset_from(Zac.name, Zac.password, Zac.mainaddress(), Frank.mainaddress(), Zac.asset_symbol, 0)
        self.assertEqual(ec, 5002, message)

    def test_A_burn(self):
        # account password match error
        ec, message = mvs_rpc.burn(Zac.name, Zac.password + '1', Zac.asset_symbol, 100)
        self.assertEqual(ec, 1000, message)

