import unittest
import utils.mvs_rpc as mvs_rpc

from Roles import Zac, Alice, Bob

class TestSendETP(unittest.TestCase):
    roles = [ Zac, Alice, Bob ]

    def setUp(self):
        for role in self.roles:
            result, message = role.create()
            self.assertEqual(result, 0, message)

    def tearDown(self):
        for role in self.roles:
            result, message = role.delete()
            self.assertEqual(result, 0, message)

    def test_0_send(self):
        '''
        Alice send etp to Zac. * 200 times
        '''
        pass