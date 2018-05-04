from TestCase.MVSTestCase import *
import MOCs

class TestSendETP(MVSTestCaseBase):
    def test_0_send(self):
        '''
        Alice send etp to Zac
        '''
        specific_amout = 123456
        specific_fee=12345
        specific_desc="test_0_send"
        ec, message = mvs_rpc.send(Alice.name, Alice.password, Zac.mainaddress(), specific_amout, fee=specific_fee, desc=specific_desc)
        self.assertEqual(ec, 0, message)

        Alice.mining()
        hash = message["transaction"]["hash"]
        ec, message = mvs_rpc.gettx(hash)
        self.assertEqual(ec, 0, message)

        tx = MOCs.Transaction.from_json(message)

        self.assertEqual(tx.hash, hash)
        self.assertEqual(tx.version, '2')
        self.assertNotEqual(len(tx.inputs), 0)

        sum_payment = 0
        for i in tx.inputs:
            ec, message = mvs_rpc.gettx(i.previous_output.hash)
            self.assertEqual(ec, 0, message)
            prev_tx = MOCs.Transaction.from_json(message)
            o = prev_tx.outputs[i.previous_output.index]
            self.assertEqual(o.index, i.previous_output.index)
            self.assertEqual(o.attachment.type, 'etp')
            self.assertIn(o.address, Alice.addresslist)
            sum_payment += o.value

        self.assertEqual(tx.outputs[0].index, 0)
        self.assertEqual(tx.outputs[0].address, Zac.mainaddress())
        self.assertEqual(tx.outputs[0].attachment.type, 'etp')
        self.assertEqual(tx.outputs[0].value, specific_amout)

        self.assertEqual(tx.outputs[1].index, 1)
        self.assertEqual(tx.outputs[1].address, Zac.mainaddress())
        self.assertEqual(tx.outputs[1].attachment.type, 'message')
        self.assertEqual(tx.outputs[1].attachment.content, specific_desc)
        self.assertEqual(tx.outputs[1].value, 0)

        if sum_payment > (specific_amout + specific_fee):
            self.assertEqual(tx.outputs[2].index, 2)
            self.assertEqual(tx.outputs[2].value, sum_payment - (specific_amout + specific_fee))
            self.assertIn(tx.outputs[2].address, Alice.addresslist)
            self.assertEqual(tx.outputs[2].attachment.type, 'etp')

    def test_1_sendfrom(self):
        '''
        Alice send etp to Zac
        '''
        specific_amout = 123321
        specific_fee = 12321
        specific_desc = "test_1_sendfrom"

        ec, message = mvs_rpc.sendfrom(Alice.name, Alice.password, Alice.mainaddress(), Zac.mainaddress(), specific_amout, fee=specific_fee,
                                   desc=specific_desc)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        hash = message["transaction"]["hash"]
        ec, message = mvs_rpc.gettx(hash)
        self.assertEqual(ec, 0, message)

        tx = MOCs.Transaction.from_json(message)

        self.assertEqual(tx.hash, hash)
        self.assertEqual(tx.version, '2')
        self.assertNotEqual(len(tx.inputs), 0)

        sum_payment = 0
        for i in tx.inputs:
            ec, message = mvs_rpc.gettx(i.previous_output.hash)
            self.assertEqual(ec, 0, message)
            prev_tx = MOCs.Transaction.from_json(message)
            o = prev_tx.outputs[i.previous_output.index]
            self.assertEqual(o.index, i.previous_output.index)
            self.assertEqual(o.attachment.type, 'etp')
            self.assertEqual(o.address, Alice.mainaddress())
            sum_payment += o.value

        self.assertEqual(tx.outputs[0].index, 0)
        self.assertEqual(tx.outputs[0].address, Zac.mainaddress())
        self.assertEqual(tx.outputs[0].attachment.type, 'etp')
        self.assertEqual(tx.outputs[0].value, specific_amout)

        self.assertEqual(tx.outputs[1].index, 1)
        self.assertEqual(tx.outputs[1].address, Zac.mainaddress())
        self.assertEqual(tx.outputs[1].attachment.type, 'message')
        self.assertEqual(tx.outputs[1].attachment.content, specific_desc)
        self.assertEqual(tx.outputs[1].value, 0)

        if sum_payment > (specific_amout + specific_fee):
            self.assertEqual(tx.outputs[2].index, 2)
            self.assertEqual(tx.outputs[2].value, sum_payment - (specific_amout + specific_fee))
            self.assertEqual(tx.outputs[2].address, Alice.mainaddress())
            self.assertEqual(tx.outputs[2].attachment.type, 'etp')

    def test_2_sendmore(self):
        '''
        Alice send etp to Zac's multi address
        '''
        receivers = {
            Zac.addresslist[0]: 100000,
            Zac.addresslist[1]: 100001,
            Zac.addresslist[2]: 100002,
            Zac.addresslist[3]: 100003,
            Zac.addresslist[4]: 100004,
            Zac.addresslist[5]: 100005,
        }
        specific_fee = 12421
        ec, message = mvs_rpc.sendmore(Alice.name, Alice.password, receivers, Alice.addresslist[1], specific_fee)
        self.assertEqual(ec, 0, message)
        Alice.mining()
        hash = message["transaction"]["hash"]
        ec, message = mvs_rpc.gettx(hash)
        self.assertEqual(ec, 0, message)

        tx = MOCs.Transaction.from_json(message)

        self.assertEqual(tx.hash, hash)
        self.assertEqual(tx.version, '2')
        self.assertNotEqual(len(tx.inputs), 0)

        sum_payment = 0
        for i in tx.inputs:
            ec, message = mvs_rpc.gettx(i.previous_output.hash)
            self.assertEqual(ec, 0, message)
            prev_tx = MOCs.Transaction.from_json(message)
            o = prev_tx.outputs[i.previous_output.index]
            self.assertEqual(o.index, i.previous_output.index)
            self.assertEqual(o.attachment.type, 'etp')
            self.assertIn(o.address, Alice.addresslist)
            sum_payment += o.value

        max_output = len(receivers)
        addr2value = {}
        for i in range(max_output):
            self.assertEqual(tx.outputs[i].index, i)
            #self.assertEqual(tx.outputs[i].address, Zac.addresslist[i])
            self.assertEqual(tx.outputs[i].attachment.type, 'etp')
            #self.assertEqual(tx.outputs[i].value, receivers[Zac.addresslist[i]])
            addr2value[tx.outputs[i].address] = tx.outputs[i].value
        self.assertEqual(receivers, addr2value, str(addr2value))

        total_out = sum([receivers[i] for i in receivers])
        if sum_payment > total_out:
            self.assertEqual(tx.outputs[ max_output ].index, max_output)
            self.assertEqual(tx.outputs[ max_output ].value, sum_payment - (total_out + specific_fee))
            self.assertEqual(tx.outputs[ max_output ].address, Alice.addresslist[1])
            self.assertEqual(tx.outputs[ max_output ].attachment.type, 'etp')