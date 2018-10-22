from pybitcoin.transactions.scripts import script_to_hex
from pybitcoin.hash import bin_double_sha256, bin_hash160
from pybitcoin.address import bin_hash160_to_address
template = '''
OP_IF
  OP_2 %(Alice)s %(Bob)s OP_2 OP_CHECKMULTISIG
OP_ELSE
  %(Sequence)s OP_CHECKSEQUENCEVERIFY
  %(Alice)s OP_CHECKSIG
OP_ENDIF
'''

contract = template % {'Alice' : '02578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573',
            'Bob' : '0380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d11',
            'Sequence' : '64'}

template2 = '''
304402205bfeabd45f247712895239e3d10fad6bd2744074190ad287e1ad7a6475ef51b6022058743b1a1809e1fead5807cd3dd9866a7d9ce782f21b1faa8869d543543a5cc201
304402201728b40dd64ad8e011109273bf2250d3864d55b5e5364ee1cbb166fd50bb834002207c17913f41d42b252519162d494d716719268db7658213204286f57fdd18fc7401
OP_1
OP_PUSHDATA1
63522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b32102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68
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
    print '-' * 80
    print compile(template2)