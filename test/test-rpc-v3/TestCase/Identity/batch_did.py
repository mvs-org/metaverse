from utils import common
from TestCase.MVSTestCase import *

class TestDIDBatch(MVSTestCaseBase):
    def test_0_registerdid_batch(self):
        # create 5000 account
        # send 1 etp to each account
        # issue a did for each account
        now = common.get_timestamp()

        def get_account_name(i, j):
            return "Account_%s_%s" % (i, j)
        def get_did_symbol(i, j):
            return "tempdid_%s_%s.%s" % (i, j, now)

        lastwords = []
        addresses = []

        batch_amount_i, batch_amount_j = 200, 10 # total 2000

        def get_lastword(i, j):
            return lastwords[i * batch_amount_j + j]

        def get_address(i, j):
            return addresses[i * batch_amount_j + j]

        def get_did_count():
            ec, message = mvs_rpc.list_dids()
            self.assertEqual(ec, 0)
            return message['total_count']

        try:
            for i in xrange(batch_amount_i):
                receivers = {}
                for j in xrange(batch_amount_j):
                    ec, (lastword, address) = mvs_rpc.easy_new_account(get_account_name(i, j), "123456")
                    self.assertEqual(ec, 0)
                    lastwords.append(lastword)
                    addresses.append( address )

                    receivers[address] = 10**8 # 1 etp for each
                Alice.sendmore_etp(receivers)
                Alice.mining()

            previous = get_did_count()
            for i in xrange(batch_amount_i):
                for j in xrange(batch_amount_j):
                    ec, message = mvs_rpc.register_did(get_account_name(i, j), "123456", get_address(i, j), get_did_symbol(i, j))
                    self.assertEqual(ec, 0)

            current = 0
            mine_round = 0
            while current < previous + batch_amount_i*batch_amount_j:
                Alice.mining()
                current = get_did_count()
                mine_round += 1

            self.assertEqual(mine_round, 2)

        finally:
            for i in xrange(batch_amount_i):
                for j in xrange(batch_amount_j):
                    ec, message = mvs_rpc.delete_account(get_account_name(i, j), "123456", get_lastword(i, j))
                    self.assertEqual(ec, 0, message)
