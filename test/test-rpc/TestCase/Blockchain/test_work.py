'''
dependency: pyethereum

sudo apt-get install libssl-dev build-essential automake pkg-config libtool libffi-dev libgmp-dev libyaml-cpp-dev
git clone https://github.com/ethereum/pyethereum/
cd pyethereum
python setup.py install

bugfix 2018-05-02:
sudo pip install rlp=0.6.0    // the latest rlp does not work!
'''
import unittest
from Roles import Alice
from utils.mvs_rpc import set_miningaccount, eth_get_work, eth_submit_work

class TestWork(unittest.TestCase):
    def setUp(self):
        result, message = Alice.create()
        self.assertEqual(result, 0, message)

    def tearDown(self):
        result, message = Alice.delete()
        self.assertEqual(result, 0, message)

    def test_1_no_mining_account(self):
        Alice.mining()
