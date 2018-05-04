import MOCs
import mvs_rpc

def validate_tx(checker, tx_hash, from_who, to_who, amount, fee, desc=None):
    '''

    :param checker: unittest instance
    :param tx_hash:
    :param from_who: role instance
    :param to_who: role instance
    :param amount:
    :param fee:
    :param desc:
    :return:
    '''
    tx_type = 'etp'
    tx = MOCs.Transaction.from_hash(tx_hash)

    checker.assertEqual(tx.hash, tx_hash)
    checker.assertEqual(tx.version, '2')
    checker.assertNotEqual(len(tx.inputs), 0)

    sum_payment = 0
    for i in tx.inputs:
        ec, message = mvs_rpc.gettx(i.previous_output.hash)
        checker.assertEqual(ec, 0, message)
        prev_tx = MOCs.Transaction.from_json(message)
        o = prev_tx.outputs[i.previous_output.index]
        checker.assertEqual(o.index, i.previous_output.index)
        checker.assertEqual(o.attachment.type, tx_type)
        checker.assertIn(o.address, from_who.addresslist)
        sum_payment += o.value

    output_index = 0

    checker.assertEqual(tx.outputs[output_index].index, output_index)
    checker.assertEqual(tx.outputs[output_index].address, to_who.mainaddress())
    checker.assertEqual(tx.outputs[output_index].attachment.type, tx_type)
    checker.assertEqual(tx.outputs[output_index].value, amount)

    output_index += 1

    if desc:
        checker.assertEqual(tx.outputs[output_index].index, output_index)
        checker.assertEqual(tx.outputs[output_index].address, to_who.mainaddress())
        checker.assertEqual(tx.outputs[output_index].attachment.type, 'message')
        checker.assertEqual(tx.outputs[output_index].attachment.content, desc)
        checker.assertEqual(tx.outputs[output_index].value, 0)

        output_index += 1

    if sum_payment > (amount + fee):
        checker.assertEqual(tx.outputs[output_index].index, output_index)
        checker.assertEqual(tx.outputs[output_index].value, sum_payment - (amount + fee))
        checker.assertIn(tx.outputs[output_index].address, from_who.addresslist)
        checker.assertEqual(tx.outputs[output_index].attachment.type, tx_type)

        output_index += 1

def validate_asset_tx():
    pass