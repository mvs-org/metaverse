import os, sys
sys.path.append('..')
import unittest
import utils.mvs_rpc as mvs_rpc
import utils.common as common


class TestSend(unittest.TestCase):
    account = "mvs_test"
    password = "tkggfngu"
    mnemonic = "alarm void rib clean stay suggest govern stove engage fury will mutual baby funny empower raccoon cement federal raise rocket fashion merit design cube".split()
    light_wallet_keystore = os.path.abspath('../resource/keystore/mvs_tkggfngu.json')
    addresses = [
        "MTU7t8AZRazN2t5p3dHRF9k6b2ibisxZxi",
        "MSoq2Ec8icWX9Ur7pSqK8mfi14XxRwPBPC",
        "MMfZsZckHJ4qskhfhzAqbPTnWfeB6bthV7",
        "MCbAqMwGMeWWpbvmfjQ8ETy6rSgiApgTLc",
        "M8QsckJYcy7FdQpzswJt49YFXkzCgqRQRw",
        "MJo4yAyfd8YTsmBKFcdmPKm9p4PcrB7gmo",
        "MFK1qPdnRs3CR6QNMBz8Fau1ns1Vc1Usi9",
        "MEFFdkvbcjMo7jh3oe2eSFoB1e217Aut6U",
        "MA1Wo8UmTxxErUfUSjrVd8zKWumfzbLnSw",
        "MBZFKYRwXhb6QrWpmxQcALJWBZfmMzMFUW"
    ]

    did_symbol = "TEST.MVS.AUTOCASE"
    max_round = 3000

    def setUp(self):
        '''
        '''
        result, message = mvs_rpc.import_keyfile(
            self.account,
            self.password,
            self.light_wallet_keystore)
        self.assertEqual(result, True, message)

        result, message = mvs_rpc.set_miningaccount(
            self.account,
            self.password,
            self.addresses[0]
        )
        self.assertEqual(result, True, message)

        result, message = mvs_rpc.issue_did(self.account, self.password, self.addresses[1], self.did_symbol)
        if not result:
            self.assertEqual(message['code'], 7002, message)


    def tearDown(self):
        result, message = mvs_rpc.delete_account(
            self.account,
            self.password,
            self.mnemonic[-1])
        self.assertEqual(result, True, message)

    def tes1t_1_send_3000_times(self):
        '''
         call send 3000 times to check the average time used.
        '''
        from_ = self.addresses[0]
        to_ = "MEvuZLD5yMaaV867YQR9auCuxLVcb6Jso9"#self.addresses[1]
        #1. check if we have more than 3 etp

        #2. create 3000 tx, each transfer 0.001 etp with fee 0.0001 etp.
        for i in xrange(self.max_round):
            print "batch <%d/%d>" % (i, self.max_round)
            result, message = mvs_rpc.sendfrom(self.account, self.password, from_, to_, 100000, 10000, "batch <%d/%d>" % (i, self.max_round))
            self.assertEqual(result, True, message)
        self.parse_debuglog()

    def test_2_didsend_3000_times(self):
        from_ = self.did_symbol#self.addresses[0]
        to_ = "MEvuZLD5yMaaV867YQR9auCuxLVcb6Jso9"  # self.addresses[1]
        # 1. check if we have more than 3 etp

        # 2. create 3000 tx, each transfer 0.001 etp with fee 0.0001 etp.
        for i in xrange(self.max_round):
            print "batch <%d/%d>" % (i, self.max_round)
            result, message = mvs_rpc.didsend(self.account, self.password, 100000, to_,  from_, 10000,
                                               "batch <%d/%d>" % (i, self.max_round))
            self.assertEqual(result, True, message)
        self.parse_debuglog()

    def parse_debuglog(self):
        return
        file = '/home/czp/.metaverse/debug.log'
        #20180424T105214 DEBUG [TX_SEND] exec tx batch <0/3000>cost: 9
        import re
        pattern = "\d{8}T\d{6} DEBUG \\[TX_SEND\\] exec tx batch <(\d+)/3000>cost: (\d+)"
        start = 'exec tx begin'
        end = 'exec tx end'
        f = open(file)
        for line in f:
            m = re.search(pattern, line)
            if m:
                seq, cost = m.groups()
                print seq, cost
        f.close()