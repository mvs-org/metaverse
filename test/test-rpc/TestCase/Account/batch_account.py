from utils import common, database
from TestCase.MVSTestCase import *

class TestAccount(MVSTestCaseBase):
    roles = ()
    need_mine = False
    def test_0_new_account(self):
        '''create new account * 5000'''
        account_table_file = '/home/%s/.metaverse/mainnet/account_table' % common.get_username()
        origin_payload_size = database.get_payload_size(account_table_file)

        batch_amount = 5000
        lastwords = []
        for i in xrange(batch_amount):
            ec, message = mvs_rpc.new_account("Account_%s" % i, "123456")
            self.assertEqual(ec, 0, message)
            lastwords.append( message[-1] )
        try:
            current_payload_size = database.get_payload_size(account_table_file)
            # each simple account record size < 300, but when getnew address, the account record will be create twice, so 600 is the reasonable record size.
            self.assertGreater(600 * 5000, current_payload_size - origin_payload_size, "each account record size shall be less than 600.")
        finally:
            for i in xrange(batch_amount):
                ec, message = mvs_rpc.delete_account("Account_%s" % i, "123456", lastwords[i])
                self.assertEqual(ec, 0, message)
