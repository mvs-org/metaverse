import unittest
from utils import mvs_rpc, common, code
from Roles import Alice, Bob, Cindy, Dale, Eric, Frank, Zac

class MVSTestCaseBase(unittest.TestCase):
    roles = (Alice, Bob, Cindy, Dale, Eric, Frank, Zac)
    need_mine = True
    def setUp(self):
        for role in self.roles:
            try:
                # check if the role exists by get_balance
                role.delete()
            finally:
                result, message = role.create()
                self.assertEqual(result, 0, message)

        # register did for role A~F, if not registered
        for role in self.roles[:-1]:
            ec, message = mvs_rpc.list_dids(role.name, role.password)
            if ec == 0 and message:
                pass
            else:
                if role != Alice:
                    Alice.send_etp(role.mainaddress(), 10 ** 8)
                    Alice.mining()
                ec, message = role.register_did()
                self.assertEqual(ec, 0, "error: {}, message: {}".format(ec, message))
                Alice.mining()

        if self.need_mine:
            #mine to clear the tx pool
            Alice.mining()

            #record current height
            _, (hash, height) = mvs_rpc.getblockheader()
            print "current block height=[%s], hash=[%s]" % (height, hash)

    def tearDown(self):
        pass
        #for role in self.roles:
        #    result, message = role.delete()
        #    self.assertEqual(result, 0, message)

    def checkResponseKeys(self, result, expect_keys):
        if result != None and isinstance(result, dict):
            expect_keys.sort()
            actual_keys = result.keys()
            actual_keys.sort()
            self.assertEqual(actual_keys, expect_keys)

class MultiSigDIDTestCase(MVSTestCaseBase):
    def setUp(self):
        MVSTestCaseBase.setUp(self)
        self.group_ABC = [Alice, Bob, Cindy]
        self.group_DEF = [Dale, Eric, Frank]
        self.addr_ABC = common.create_multisig_address(self.group_ABC, 2)
        self.addr_DEF = common.create_multisig_address(self.group_DEF, 2)

        # register did if not registered
        self.did_ABC = "Alice.Bob.Cindy@DIID"
        self.did_DEF = "Dale.Eric.Frank@DIID"

        for roles, addr, attr_name in [(self.group_ABC, self.addr_ABC, "did_ABC"), (self.group_DEF, self.addr_DEF, "did_DEF")]:
            ec, message = mvs_rpc.list_dids(roles[-1].name, roles[-1].password)
            self.assertEqual(ec, 0, message)
            for did_info in message:
                if did_info["address"] == addr:
                    setattr(self, attr_name, did_info["symbol"])
                    break
            else:
                # not issued
                Alice.send_etp(addr, 10**8)
                Alice.mining()

                did_symbol = getattr(self, attr_name)

                ec, tx = mvs_rpc.register_did(roles[0].name, roles[0].password, addr, did_symbol)
                self.assertEqual(ec, 0, tx)
                ec, tx = mvs_rpc.sign_multisigtx(roles[1].name, roles[1].password, tx, True)
                self.assertEqual(ec, 0, tx)
                Alice.mining()

class ForkTestCase(MVSTestCaseBase):
    remote_ip = "10.10.10.28"
    remote_ctrl = None
    @classmethod
    def setUpClass(cls):
        cls.remote_ctrl = mvs_rpc.RemoteCtrl(cls.remote_ip)

    def setUp(self):
        '''import Alice account to the remote'''
        MVSTestCaseBase.setUp(self)
        self.partion_height = -1
        try:
            with open(Alice.keystore_file) as f:
                ec, message = self.remote_ctrl.import_keyfile(Alice.name, Alice.password, "any", f.read() )
                # it still works if the account Alice already exist in remote wallet
                #self.assertEqual(ec, 0, message)
        except :
            print 'unable to connect remote url:'+self.remote_ip


    def tearDown(self):
        try:
            self.remote_ctrl.delete_account(Alice.name, Alice.password, Alice.lastword())

            ec, message = mvs_rpc.add_node( self.remote_ip + ':5251')
            self.assertEqual(ec, 0, message)
        except :
            print 'unable to connect remote url:'+self.remote_ip

        MVSTestCaseBase.tearDown(self)

    def make_partion(self):
        '''make the p2p network partion into 2 seperate ones.'''
        # record current block height
        ec, message = mvs_rpc.get_info()
        self.assertEqual(ec, 0, message)
        self.partion_height = message[0]

        ec, peers = mvs_rpc.getpeerinfo()
        for peer in peers:
            ec, message = mvs_rpc.ban_node( peer)
            self.assertEqual(ec, 0, message)

    def remote_ming(self, times):
        mining = mvs_rpc.remote_call(self.remote_ip, Alice.mining)
        mining(times)

        get_info = mvs_rpc.remote_call(self.remote_ip, mvs_rpc.get_info)
        ec, message = get_info()
        self.assertEqual(ec, 0, message)
        return message[0] # expect hight

    def fork(self):
        try:
            ming_round = 6
            expect_hight = self.remote_ming(ming_round)

            ec, message = mvs_rpc.add_node( self.remote_ip+':5251')
            self.assertEqual(ec, 0, message)
            import time
            new_height = 0

            # wait until the fork complete
            timeout = 300
            while  new_height < expect_hight:#self.partion_height + ming_round:
                time.sleep(1)
                ec, message = mvs_rpc.get_info()
                self.assertEqual(ec, 0, message)
                new_height = message[0]
                timeout -= 1
                self.assertGreater(timeout, 0, "wait fork timeout error!")
        except :
            print 'unable to connect remote url:'+self.remote_ip

