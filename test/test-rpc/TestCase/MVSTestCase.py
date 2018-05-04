import unittest
from utils import mvs_rpc
from Roles import Alice, Bob, Cindy, Dale, Eric, Frank, Zac

class MVSTestCaseBase(unittest.TestCase):
    roles = (Alice, Bob, Cindy, Dale, Eric, Frank, Zac)

    def setUp(self):
        for role in self.roles:
            result, message = role.create()
            self.assertEqual(result, 0, message)
        #mine to clear the tx pool
        Alice.mining()

        #record current height
        _, (hash, height) = mvs_rpc.getblockheader()
        print "current block height=[%s], hash=[%s]" % (height, hash)

    def tearDown(self):
        for role in self.roles:
            result, message = role.delete()
            self.assertEqual(result, 0, message)
        