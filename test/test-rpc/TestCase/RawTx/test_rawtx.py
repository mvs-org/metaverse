from TestCase.MVSTestCase import *

# error codes
command_params_exception = 1021
argument_legality_exception = 2003
toaddress_invalid_exception = 4012
fromaddress_invalid_exception = 4015
asset_symbol_length_exception = 5011
asset_lack_exception = 5001
asset_exchange_poundage_exception = 5005


class TestRawTx(MVSTestCaseBase):

    def test_0_create_rawtx_boundary(self):
        senders = {Alice.mainaddress()}
        single_receiver = {Bob.mainaddress() : 10}
        multi_receivers = {Bob.mainaddress() : 10, Cindy.mainaddress() : 20}

        # invalid type
        ec, message = mvs_rpc.create_rawtx(4, senders, single_receiver);
        self.assertEqual(ec, argument_legality_exception, message)

        # invalid senders address
        ec, message = mvs_rpc.create_rawtx(0, {Alice.mainaddress() + "1"}, single_receiver,);
        self.assertEqual(ec, fromaddress_invalid_exception, message)

        # invalid receiver address
        ec, message = mvs_rpc.create_rawtx(0,
            {Alice.mainaddress()},
            {Bob.mainaddress()+"1" : 10},
            mychange=Alice.mainaddress());
        self.assertEqual(ec, toaddress_invalid_exception, message)

        # invalid receiver amount
        ec, message = mvs_rpc.create_rawtx(0,
            {Alice.mainaddress()},
            {Bob.mainaddress() : 0},
            mychange=Alice.mainaddress());
        self.assertEqual(ec, argument_legality_exception, message)

        # invalid mychange address
        ec, message = mvs_rpc.create_rawtx(0, senders, single_receiver, mychange=Alice.mainaddress()+"1");
        self.assertEqual(ec, toaddress_invalid_exception, message)

        # invalid symbol for deposit etp
        ec, message = mvs_rpc.create_rawtx(1, senders, single_receiver, symbol="ABC");
        self.assertEqual(ec, argument_legality_exception, message)

        # invalid multi receivers for deposit etp
        ec, message = mvs_rpc.create_rawtx(1, senders, multi_receivers);
        self.assertEqual(ec, argument_legality_exception, message)

        # invalid symbol for transfer asset
        ec, message = mvs_rpc.create_rawtx(3, senders, single_receiver, symbol="");
        self.assertEqual(ec, asset_symbol_length_exception, message)

        ec, message = mvs_rpc.create_rawtx(3, senders, single_receiver, symbol="x"*65);
        self.assertEqual(ec, asset_symbol_length_exception, message)

        # no asset exist
        ec, message = mvs_rpc.create_rawtx(3, senders, single_receiver, symbol="x"*64);
        self.assertEqual(ec, asset_lack_exception, message)

        # invalid fee
        ec, message = mvs_rpc.create_rawtx(0, senders, multi_receivers, fee=0);
        self.assertEqual(ec, asset_exchange_poundage_exception, message)

        ec, message = mvs_rpc.create_rawtx(0, senders, multi_receivers, fee=10**4 -1);
        self.assertEqual(ec, asset_exchange_poundage_exception, message)

        ec, message = mvs_rpc.create_rawtx(0, senders, multi_receivers, fee=10000000001);
        self.assertEqual(ec, asset_exchange_poundage_exception, message)


    def test_1_create_rawtx_transfer_etp(self):
        senders = {Alice.mainaddress()}
        single_receiver = {Bob.mainaddress() : 10}
        multi_receivers = {Bob.mainaddress() : 10, Cindy.mainaddress() : 20}

        ec, message = mvs_rpc.create_rawtx(0, senders, single_receiver, mychange=Alice.mainaddress());
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()

        ec, message = mvs_rpc.create_rawtx(0, senders, multi_receivers, mychange=Alice.mainaddress());
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()


    def test_2_create_rawtx_deposit_etp(self):
        senders = {Alice.mainaddress()}
        single_receiver = {Bob.mainaddress() : 10}
        multi_receivers = {Bob.mainaddress() : 10, Cindy.mainaddress() : 20}

        ec, message = mvs_rpc.create_rawtx(1, senders, single_receiver, mychange=Alice.mainaddress());
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()


    def test_3_create_rawtx_transfer_asset(self):
        senders = {Alice.mainaddress()}
        single_receiver = {Bob.mainaddress() : 10}
        multi_receivers = {Bob.mainaddress() : 10, Cindy.mainaddress() : 20}

        # create asset
        domain_symbol, asset_symbol = Alice.create_random_asset(secondary=-1)
        Alice.mining()

        ec, message = mvs_rpc.create_rawtx(3, senders, single_receiver, symbol=asset_symbol);
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()

        ec, message = mvs_rpc.create_rawtx(3, senders, multi_receivers, symbol=asset_symbol);
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()

    def test_4_rawtx(self):
        senders = {Alice.mainaddress()}
        single_receiver = {Bob.mainaddress() : 10}
        multi_receivers = {Bob.mainaddress() : 10, Cindy.mainaddress() : 20}

        # create rawtx
        ec, message = mvs_rpc.create_rawtx(0, senders, single_receiver, mychange=Alice.mainaddress());
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()

        rawtx = message["hex"]

        # invalid password
        ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password + '1', rawtx)
        self.assertEqual(ec, 1000, message)

        # invalid account
        ec, message = mvs_rpc.sign_rawtx(Bob.name, Bob.password, rawtx)
        self.assertEqual(ec, argument_legality_exception, message)

        # invalid transaction
        ec, message = mvs_rpc.sign_rawtx(Bob.name, Bob.password, rawtx + '1')
        self.assertEqual(ec, command_params_exception, message)

        # sign
        ec, message = mvs_rpc.sign_rawtx(Alice.name, Alice.password, rawtx)
        self.assertEqual(ec, 0, message)
        self.assertGreater(len(message["hash"]), 0)
        self.assertGreater(len(message["hex"]), 0)
        Alice.mining()

        hash2 = message["hash"]
        rawtx2 = message["hex"]

        #
        # decode rawtx
        #

        # invalid rawtx
        ec, message = mvs_rpc.decode_rawtx(rawtx + '1')
        self.assertEqual(ec, command_params_exception, message)

        # decode rawtx
        ec, message = mvs_rpc.decode_rawtx(rawtx)
        self.assertEqual(ec, 0, message)

        # decode rawtx2
        ec, message = mvs_rpc.decode_rawtx(rawtx2)
        self.assertEqual(ec, 0, message)

        transaction = message["transaction"]
        self.assertEqual(transaction["hash"], hash2, message)

        #
        # sendrawtx
        #

        # invalid rawtx
        ec, message = mvs_rpc.send_rawtx(rawtx + '1')
        self.assertEqual(ec, command_params_exception, message)

        ec, message = mvs_rpc.send_rawtx(rawtx2)
        self.assertEqual(ec, 0, message)
        Alice.mining()

        self.assertEqual(message["hash"], hash2, message)
