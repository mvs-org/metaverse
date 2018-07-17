from TestCase.MVSTestCase import *

class TestSendETP(MVSTestCaseBase):
    def test_0_send(self):
        #account password mismatch
        ec, message = mvs_rpc.send(Alice.name, Alice.password+"1", Zac.mainaddress(), 1)
        self.assertEqual(ec, 1000, message)

        # check to_address
        ec, message = mvs_rpc.send(Alice.name, Alice.password, Zac.mainaddress()+'1', 1)
        self.assertEqual(ec, 4010, message)

        # not enough balance
        ec, message = mvs_rpc.send(Zac.name, Zac.password, Alice.mainaddress(), 1)
        self.assertEqual(ec, 5302, message)

    def test_1_sendfrom(self):
        # account password mismatch
        ec, message = mvs_rpc.sendfrom(Alice.name, Alice.password + "1", Alice.mainaddress(), Zac.mainaddress(), 1)
        self.assertEqual(ec, 1000, message)

        # check from_address
        ec, message = mvs_rpc.sendfrom(Alice.name, Alice.password, Alice.mainaddress()+ "1", Zac.mainaddress(), 1)
        self.assertEqual(ec, 4015, message)

        # check to_address
        ec, message = mvs_rpc.sendfrom(Alice.name, Alice.password, Alice.mainaddress(), Zac.mainaddress() + "1", 1)
        self.assertEqual(ec, 4012, message)

        # not enough balance
        ec, message = mvs_rpc.send(Zac.name, Zac.password, Alice.mainaddress(), 1)
        self.assertEqual(ec, 5302, message)

    def test_2_sendmore(self):
        # account password mismatch
        ec, message = mvs_rpc.sendmore(Zac.name, Zac.password + "1", {Alice.mainaddress():10000, Bob.mainaddress():20000})
        self.assertEqual(ec, 1000, message)

        #validate my change address
        ec, message = mvs_rpc.sendmore(Zac.name, Zac.password, {Alice.mainaddress(): 10000, Bob.mainaddress(): 20000}, Zac.mainaddress() + '1')
        self.assertEqual(ec, 4012, message)

        #my change address does not belong to me
        #ec, message = mvs_rpc.sendmore(Zac.name, Zac.password, {Alice.mainaddress(): 10000, Bob.mainaddress(): 20000}, Alice.mainaddress())
        #self.assertEqual(ec, 4012, message)

        #not enough balance
        ec, message = mvs_rpc.sendmore(Zac.name, Zac.password, {Alice.mainaddress(): 10000, Bob.mainaddress(): 20000}, Zac.mainaddress())
        self.assertEqual(ec, 5302, message)