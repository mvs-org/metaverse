from pybitcoin.transactions.scripts import script_to_hex
from pybitcoin.hash import bin_double_sha256, bin_hash160
from pybitcoin.address import bin_hash160_to_address
template = '''
OP_IF
  OP_2 %(Alice)s %(Bob)s OP_2 OP_CHECKMULTISIG
OP_ELSE
  %(Sequence)s OP_CHECKSEQUENCEVERIFY OP_DROP
  %(Alice)s OP_CHECKSIG
OP_ENDIF
'''

contract = template % {'Alice' : '02578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573',
            'Bob' : '0380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d11',
            'Sequence' : '0a000000'}

template2 = '''
OP_0
3045022100ff054d83e4f376b6b47705b8186fd1e2b61cabe70e717f052b6bf0fd00d883ec02203adaf168c7e4b32fbd66dd2adfdd42aaf6268f5e4c736978ab6c86d4e13bfcf401
304402200eab2db325b0c95dcfed00a4554b59d3422d2eef3eed50a341da55cd83e8e06302203fc97b96df2e803dfc3113cc6ee0dd5728ced316b63dfda72c808ab48826f7e601
OP_1
OP_PUSHDATA1
63522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b3752102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68
'''

template3 = '''
304402204d21c19216cad74e780bd70e04518cf8f1a20108dc3bf79f7b218865524661ac022049b5de8a05d9b524ae6de3b4b221c856d16d4e3a51f7f19e685e7fc33b51abac01
OP_0
OP_PUSHDATA1
63522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b3752102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68
'''


def compile(ct):
    ct = ct.split()
    #for i in ct:
    #    print i, '->', script_to_hex(i)
    return script_to_hex(' '.join(ct))

if __name__ == '__main__':
    script = compile(contract)
    script_hash = bin_hash160(script, hex_format=True)
    p2sh = bin_hash160_to_address(script_hash, 5)
    print p2sh
    print script
    print '-' * 80
    print compile(template3)