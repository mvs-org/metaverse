from TestCase.MVSTestCase import *
from tools import data_base
import hashlib

def get_offset_by_index(index):
    with open( data_base.mainnet_dir + '/account_remark_table' ) as fr:
        fr.seek(4 + 8*index)
        return data_base.str2int( fr.read(8) )


class TestAccountRemark(MVSTestCaseBase):
    def test_0_account_index(self):
        at = data_base.account_table()
        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 0) # normal account
        # allocate index for account by making 1 transaction
        expect_remark = "test_0_account_index"*100

        ec, message = mvs_rpc.send(Alice.name, Alice.password, Alice.addresslist[1], 10 ** 8, remark=expect_remark)
        self.assertEqual(ec, 0, message)

        expect_tx_hash = message["transaction"]["hash"]

        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 128)  # normal account & has account index
        origin_account_index = result['account_index']
        Alice.mining()
        ec, (height, _)  = mvs_rpc.get_info()
        self.assertEqual(ec, 0, height)

        ec, message = mvs_rpc.listtxs(Alice.name, Alice.password, height=(height, height+1))
        self.assertEqual(ec, 0, message)
        for tx in message["transactions"]:
            if tx["hash"] == expect_tx_hash:
                self.assertEqual(tx["remark"], expect_remark)
                break
        else:
            self.assertEqual(0,1,"[%s] not found!" % expect_tx_hash)

        # check table
        self.assertNotEqual(0xFFFFFFFFFFFFFFFF, get_offset_by_index(origin_account_index))

        # delete account to release that index
        Alice.delete()
        try:
            # check table
            self.assertEqual(0xFFFFFFFFFFFFFFFF, get_offset_by_index(origin_account_index))
        finally:
            Alice.create()


    def test_1_account_normalXmultisig(self):
        # account without index
        # convert normal account from/to multisig one.
        at = data_base.account_table()
        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 0)  # normal account

        address = Alice.new_multisigaddress("test_1_account_normalXmultisig", [Bob, Cindy], 2)
        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 1)  # multisig account

        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password, address)
        self.assertEqual(ec, 0, message)

        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 0)  # normal account

        # account with index
        # convert normal account from/to multisig one.
        expect_remark = "test_1_account_normalXmultisig"
        ec, message = mvs_rpc.send(Alice.name, Alice.password, Alice.addresslist[1], 10 ** 8, remark=expect_remark)
        self.assertEqual(ec, 0, message)

        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 128)  # normal account with acount index

        address = Alice.new_multisigaddress("test_1_account_normalXmultisig", [Bob, Cindy], 2)
        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 129)  # multisig account with acount index

        ec, message = mvs_rpc.delete_multisig(Alice.name, Alice.password, address)
        self.assertEqual(ec, 0, message)

        result = at.query(hashlib.sha256(Alice.name).digest(), data_base.mainnet_dir)
        self.assertEqual(result['name'], Alice.name)
        self.assertEqual(result['type'], 128)  # normal account

    def test_2_dumpXimport_remark(self):
        # account with remarks
        # dumpkeyfile with remarks
        expect_remark = "test_2_dumpXimport_remark"
        ec, message = mvs_rpc.send(Alice.name, Alice.password, Alice.addresslist[1], 10 ** 8, remark=expect_remark)
        self.assertEqual(ec, 0, message)
        expect_tx_hash = message["transaction"]["hash"]

        ec, message = mvs_rpc.dump_keyfile_v2(Alice.name, Alice.password, Alice.lastword())
        self.assertEqual(ec, 0, message)

        self.assertEqual(message['remarks'], {expect_tx_hash:expect_remark})

        # and then import it back
        # check if these remarks work well
        pass

