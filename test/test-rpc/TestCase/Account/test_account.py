import unittest
from Roles import Zac

class TestAccount(unittest.TestCase):
    def setUp(self):
        result, message = Zac.create()
        self.assertEqual(result, 0, message)

    def tearDown(self):
        result, message = Zac.delete()
        self.assertEqual(result, 0, message)

    def test_nothing(self):
        pass