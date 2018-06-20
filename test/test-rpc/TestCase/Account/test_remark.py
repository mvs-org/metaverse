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

        # send
        expect_remark = remark_template % "send"
        ec, message = mvs_rpc.send(Alice.name, Alice.password, Alice.addresslist[1], 10 ** 8, remark=expect_remark)
        self.assertEqual(ec, 0, message)
        expect_tx_hash = message["transaction"]["hash"]
        expect[expect_tx_hash] = expect_remark

        # sendfrom
        expect_remark = remark_template % "sendfrom"
        ec, message = mvs_rpc.sendfrom(Alice.name, Alice.password, Alice.mainaddress(), Alice.addresslist[1], 10 ** 8, remark=expect_remark)
        self.assertEqual(ec, 0, message)
        expect_tx_hash = message["transaction"]["hash"]
        expect[expect_tx_hash] = expect_remark

        # didsend
        expect_remark = remark_template % "didsend"
        ec, message = mvs_rpc.didsend(Alice.name, Alice.password, Bob.did_symbol, 10 ** 8, remark=expect_remark)
        self.assertEqual(ec, 0, message)
        expect_tx_hash = message["transaction"]["hash"]
        expect[expect_tx_hash] = expect_remark

        # didsendfrom
        expect_remark = remark_template % "didsendfrom"
        ec, message = mvs_rpc.didsend_from(Alice.name, Alice.password, Alice.did_symbol, Bob.did_symbol, 10 ** 8, remark=expect_remark)
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

