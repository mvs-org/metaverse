#!/usr/bin/python
# -*- coding: utf-8 -*-
import random
import unittest
import utils.mvs_rpc as mvs_rpc

from Roles import Zac

class TestAssetBoundary(unittest.TestCase):
    roles = [ Zac ]

    def setUp(self):
        for role in self.roles:
            result, message = role.create()
            self.assertEqual(result, 0, message)

    def tearDown(self):
        for role in self.roles:
            result, message = role.delete()
            self.assertEqual(result, 0, message)

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

    def test_1_create_asset(self):
        #account password match error
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password + '1', Zac.asset_symbol, 100)
        self.assertEqual(ec, 1000, message)

        #aasset symbol can not be empty.
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "", 100)
        self.assertEqual(ec, 5011, message)

        #asset symbol length must be less than 64.
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 65, 100)
        self.assertEqual(ec, 5011, message)

        #asset description length must be less than 64.
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 64, 100, "x"*65)
        self.assertEqual(ec, 5007, message)

        #secondaryissue threshold value error, is must be -1 or in range of 0 to 100.
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 64, 100, "x" * 64, rate=-2)
        self.assertEqual(ec, 5016, message)

        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 64, 100, "x" * 64, rate=101)
        self.assertEqual(ec, 5016, message)

        #asset decimal number must less than 20.
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 64, 100, "x" * 64, rate=0, decimalnumber=-1)
        self.assertEqual(ec, 5002, message)
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 64, 100, "x" * 64, rate=0, decimalnumber=20)
        self.assertEqual(ec, 5002, message)

        #volume must not be zero.
        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "x" * 64, 0, "x" * 64, rate=0, decimalnumber=19)
        self.assertEqual(ec, 2003, message)
        #contain sensitive words

        ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, "JZM.baga", 10, "x" * 64, rate=0, decimalnumber=19)
        self.assertEqual(ec, 5012, message)

        exist_symbol = self.getExistAssetSymbol()
        if exist_symbol:
            ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, exist_symbol, 10, "x" * 64, rate=0, decimalnumber=19)
            self.assertEqual(ec, 5009, message)

    def test_2_issue_asset(self):
        ten_etp = 10 * (10 ** 8)
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

        if isDomainExist("ALICE."):
            asset_symbol = Zac.asset_symbol.replace("ZAC.", "ALICE.")
            # domain cert not belong to current account
            ec, message = mvs_rpc.create_asset(Zac.name, Zac.password, asset_symbol, 100)
            self.assertEqual(ec, 0, message)

            ec, message = mvs_rpc.issue_asset(Zac.name, Zac.password, asset_symbol, ten_etp)
            self.assertEqual(ec, 5017, message)