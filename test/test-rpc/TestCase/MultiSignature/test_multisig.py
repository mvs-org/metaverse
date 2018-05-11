from TestCase.MVSTestCase import *

class TestMultiSig(MVSTestCaseBase):
    def test_0_multisig(self):
        #create multisig addr
        addr_a = Alice.new_multisigaddress("A&B&Z", [Bob, Zac], 2)
        addr_b = Bob.new_multisigaddress("B&A&Z", [Alice, Zac], 2)
        addr_z = Zac.new_multisigaddress("Z&A&B", [Alice, Bob], 2)

        self.assertEqual(addr_a, addr_b)
        self.assertEqual(addr_a, addr_z)

        amount = 10**8
        fee = 10 ** 4

        filter_balances = lambda x: x['balance']['address'] == addr_z

        #send to multisig-addr
        Alice.send_etp(addr_z, amount)
        Alice.mining()
        ec, message = mvs_rpc.list_balances(Zac.name, Zac.password, (amount, amount))
        self.assertEqual(ec, 0 , message)

        balances = filter(filter_balances, message['balances'])
        self.assertNotEqual(balances, None, balances)
        self.assertEqual(len(balances), 1)

        self.assertEqual(balances[0]['balance']['address'], addr_z)
        self.assertEqual(balances[0]['balance']['unspent'], amount)

        #send from multisig-addr
        ec, tx = mvs_rpc.create_multisigtx(Alice.name, Alice.password, addr_a, Zac.mainaddress(), amount-fee)
        self.assertEqual(ec, 0 , tx)

        ec, message = mvs_rpc.sign_multisigtx(Bob.name, Bob.password, tx, True)
        self.assertEqual(ec, 0, message)

        Alice.mining()

        ec, message = mvs_rpc.list_balances(Zac.name, Zac.password, (0, amount))
        self.assertEqual(ec, 0, message)

        balances = filter(filter_balances, message['balances'])
        self.assertEqual(len(balances), 1)
        self.assertEqual(balances[0]['balance']['unspent'], 0)





