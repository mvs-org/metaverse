'''
test issuedid/modifydid when fork occured
'''
from utils import common
from TestCase.MVSTestCase import *

class TestFork(ForkTestCase):
    def test_0_fork_at_send(self):
        self.make_partion()
        # make transaction and mine
        Alice.send_etp(Bob.mainaddress(), 10**8)
        Alice.mining()

        self.fork()

        # check if Alice & Bob's address_did record is cleaned
        for role in [Alice, Bob]:
            ec, message = mvs_rpc.list_dids(role.name, role.password)
            self.assertEqual(ec, 0, message)

            self.assertIn({'status':"issued",
            'address':role.mainaddress(),
            'symbol':role.did_symbol}, message['dids'])


