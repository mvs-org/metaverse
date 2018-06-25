'''
dependency: pyethereum

sudo apt-get install libssl-dev build-essential automake pkg-config libtool libffi-dev libgmp-dev libyaml-cpp-dev
git clone https://github.com/ethereum/pyethereum/
cd pyethereum
python setup.py install

bugfix 2018-05-02:
sudo pip install rlp=0.6.0    // the latest rlp does not work!
'''
from TestCase.MVSTestCase import *

class TestWork(MVSTestCaseBase):
    need_mine = False
    def test_1_no_mining_account(self):
        _, prev = mvs_rpc.getblockheader()
        round_to_mine = 1
        Alice.mining(round_to_mine)
        _, curr = mvs_rpc.getblockheader()
        self.assertEqual(prev[1] + round_to_mine, curr[1])
