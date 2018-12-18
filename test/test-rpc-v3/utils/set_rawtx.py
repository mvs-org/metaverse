import sys
import struct
from binascii import b2a_hex, a2b_hex
from io import BytesIO


str2int_map = {
    1: '<B',
    2: '<H',
    4: '<L',
    8: '<Q',
}

def bytes2int(b):
    global str2int_map
    return struct.unpack(str2int_map[len(b)], b)[0]

def int2bytes(i, size):
    global str2int_map
    return struct.pack(str2int_map[size], i)

def get_varlen(l):
    if l < 0xFD:
        return int2bytes(l, 1)
    if l < 0xFFFF:
        return bytes([0xFD]) + int2bytes(l, 2)
    if l < 0xFFFFFFFF:
        return bytes([0xFE]) + int2bytes(l, 4)
    if l < 0xFFFFFFFFFFFFFFFF:
        return bytes([0xFF]) + int2bytes(l, 8)

def get_bitcoin_varlen(f):
    flag = bytes2int( f.read(1) )
    if flag < 0xFD:
        return flag
    if flag == 0xFD:
        return bytes2int(f.read(2))
    if flag == 0xFE:
        return bytes2int(f.read(4))
    if flag == 0xFF:
        return bytes2int(f.read(8))

class Input:
    utxo_hash = None
    utxo_index = None
    script = None
    sequence = None
    def __init__(self, utxo_hash, utxo_index, script, sequence):
        self.utxo_hash = utxo_hash
        self.utxo_index = utxo_index
        self.script = script
        self.sequence = sequence

class Attach:
    version = None
    type = None

    from_did = None
    to_did = None

class Output:
    amount = None
    script = None
    attach = None

    def __init__(self, amount, script, attach):
        self.amount = amount
        self.script = script
        self.attach = attach

class Transaction:
    version = None
    inputs = []

    remain_bytes = None

    outputs = []
    locktime = None

    def to_rawtx(self):
        version = int2bytes(self.version, 4)
        input_size = get_varlen( len(self.inputs) )
        inputs = b''
        for input in self.inputs:
            inputs += ( input.utxo_hash
                        + int2bytes(input.utxo_index, 4)
                        + get_varlen( len(input.script) )
                        + input.script
                        + int2bytes(input.sequence, 4) )
        ret = version + input_size + inputs + self.remain_bytes + int2bytes(self.locktime, 4)
        return str(b2a_hex(ret), 'utf-8')

    @classmethod
    def parse_rawtx(cls, rawtx):
        obj = Transaction()
        bintx = a2b_hex(rawtx)
        f = BytesIO()
        f.write(bintx)
        f.seek(0, 0)

        obj.version = bytes2int( f.read(4) )

        obj.inputs = []
        n_input = get_bitcoin_varlen( f )
        for i in range(n_input):
            # previous_output
            utxo_hash = f.read(32)
            utxo_index = bytes2int( f.read(4) )

            # scprit
            script_len = get_bitcoin_varlen( f )
            script = f.read(script_len)
            # seq
            sequence = bytes2int( f.read(4) )

            obj.inputs.append( Input(utxo_hash, utxo_index, script, sequence) )

        obj.remain_bytes = bintx[f.tell():-4]
        obj.locktime = bytes2int( bintx[-4:] )
        return obj


if __name__ == "__main__":
    rawtx = '0400000001399a63ac16940f366b337f470a8e3dbffc1f9d97be640eea0a51f3aaf995c7d4000000009200483045022100e7494be04771e174a9afba7835770d8536af96b01980668c1017e37c86ce8f3b022050a3ee68436cf8f270fadb752021ae48b6aaba59cdaf43a12923c4759221229d0147522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152aeffffffff0280969800000000001976a9148b24031888c2896cedb764012677868b5c64ef3b88ac010000000000000070235d050000000017a9145551e39156a9006ae8a8c57bc4f816b9578144f787010000000000000000000000'
    sig_script = '00483045022100ff054d83e4f376b6b47705b8186fd1e2b61cabe70e717f052b6bf0fd00d883ec02203adaf168c7e4b32fbd66dd2adfdd42aaf6268f5e4c736978ab6c86d4e13bfcf40147304402200eab2db325b0c95dcfed00a4554b59d3422d2eef3eed50a341da55cd83e8e06302203fc97b96df2e803dfc3113cc6ee0dd5728ced316b63dfda72c808ab48826f7e601514c7063522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b32102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68'
    sig_script = '47304402204d21c19216cad74e780bd70e04518cf8f1a20108dc3bf79f7b218865524661ac022049b5de8a05d9b524ae6de3b4b221c856d16d4e3a51f7f19e685e7fc33b51abac01004c7163522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b3752102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68'
    redeem_script = '63522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b3752102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68'
    #rawtx = sys.argv[1]
    t = Transaction.parse_rawtx(rawtx)
    t.inputs[0].utxo_hash = a2b_hex('3309eac76354785735b33ecb4238c28486de8d319231d40831859bc4c4352fa9')[::-1]
    t.inputs[0].utxo_index = 0
    t.inputs[0].script = a2b_hex(sig_script) #bytes([]) #
    #t.inputs[0].sequence = (1 << 22) | 100 # time
    t.inputs[0].sequence =         0 | 0x0054  # height
    ret = t.to_rawtx()
    print( ret )