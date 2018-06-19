from TestCase.MVSTestCase import *
from tools import data_base
import hashlib

def get_offset_by_index(index):
    with open( data_base.mainnet_dir + '/account_remark_table' ) as fr:
        fr.seek(4 + 8*index)
        return data_base.str2int( fr.read(8) )


class TestAccountRemark(MVSTestCaseBase):
    def test_2_dumpXimport_remark(self):
        # account with remarks
        # dumpkeyfile with remarks
        expect = {}

        remark_template = "test_2_dumpXimport_remark@%s"
        for i in range(10):
            expect_remark = remark_template % i
            ec, message = mvs_rpc.send(Alice.name, Alice.password, Alice.addresslist[1], 10 ** 8, remark=expect_remark)
            self.assertEqual(ec, 0, message)
            expect_tx_hash = message["transaction"]["hash"]

            expect[expect_tx_hash] = expect_remark

        ec, message = mvs_rpc.dump_keyfile_v2(Alice.name, Alice.password, Alice.lastword())
        self.assertEqual(ec, 0, message)

        self.assertEqual(message['remarks'], expect)
        Alice.delete()
        Alice.create()
        ec, message = mvs_rpc.dump_keyfile_v2(Alice.name, Alice.password, Alice.lastword())
        self.assertEqual(ec, 0, message)

        self.assertEqual(message['remarks'], None)

        # and then import it back
        # check if these remarks work well
        pass

