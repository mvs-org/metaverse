import unittest
from utils import mvs_rpc, common
from Roles import Alice, Bob, Cindy, Dale, Eric, Frank, Zac

class MVSTestCaseBase(unittest.TestCase):
    roles = (Alice, Bob, Cindy, Dale, Eric, Frank, Zac)
    need_mine = True
    def setUp(self):
        for role in self.roles:
            result, message = role.create()
            self.assertEqual(result, 0, message)

        while Alice.get_balance() < 1000 * (10**8):
            Alice.mining(100)

        # issue did for role A~F, if not issued
        for role in self.roles[:-1]:
            ec, message = mvs_rpc.list_dids(role.name, role.password)
            if ec == 0 and message['dids']:
                pass
            else:
                if role != Alice:
                    Alice.send_etp(role.mainaddress(), 10 ** 8)
                    Alice.mining()
                ec, message = role.issue_did()
                self.assertEqual(ec, 0, message)
                Alice.mining()

        if self.need_mine:
            #mine to clear the tx pool
            Alice.mining()

            #record current height
            _, (hash, height) = mvs_rpc.getblockheader()
            print "current block height=[%s], hash=[%s]" % (height, hash)

    def tearDown(self):
        for role in self.roles:
            result, message = role.delete()
            self.assertEqual(result, 0, message)

class MultiSigDIDTestCase(MVSTestCaseBase):
    def setUp(self):
        MVSTestCaseBase.setUp(self)
        self.group_ABC = [Alice, Bob, Cindy]
        self.group_DEF = [Dale, Eric, Frank]
        self.addr_ABC = common.create_multisig_address(self.group_ABC, 2)
        self.addr_DEF = common.create_multisig_address(self.group_DEF, 2)

        # issue did if not issued
        self.did_ABC = "Alice.Bob.Cindy@DID"
        self.did_DEF = "Dale.Eric.Frank@DID"

        for roles, addr, attr_name in [(self.group_ABC, self.addr_ABC, "did_ABC"), (self.group_DEF, self.addr_DEF, "did_DEF")]:
            ec, message = mvs_rpc.list_dids(roles[-1].name, roles[-1].password)
            self.assertEqual(ec, 0, message)
            for did_info in message["dids"]:
                if did_info["address"] == addr:
                    setattr(self, attr_name, did_info["symbol"])
                    break
            else:
                # not issued
                Alice.send_etp(addr, 10**8)
                Alice.mining()

                did_symbol = getattr(self, attr_name)

                ec, tx = mvs_rpc.issue_did(roles[0].name, roles[0].password, addr, did_symbol)
                self.assertEqual(ec, 0, tx)
                ec, tx = mvs_rpc.sign_multisigtx(roles[1].name, roles[1].password, tx, True)
                self.assertEqual(ec, 0, tx)
                Alice.mining()


