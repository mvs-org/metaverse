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
        return version + input_size + inputs + self.remain_bytes

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

        obj.remain_bytes = bintx[f.tell():]
        return obj


if __name__ == "__main__":
    rawtx = '0400000001254986aa70f0261c519f211d4697a1b325a91315064261338ab11a1ab27173f40000000000ffffffff0280969800000000001976a9148b24031888c2896cedb764012677868b5c64ef3b88ac010000000000000070235d05000000001976a914c64bd3fb0959db9212066496275e4d5a858f3e5e88ac010000000000000000000000'
    #rawtx = sys.argv[1]
    t = Transaction.parse_rawtx(rawtx)
    t.inputs[0].sequence = (1 << 22) | 333 # time
    #t.inputs[0].sequence =         0 | 0x0003  # height
    ret = t.to_rawtx()
    print( b2a_hex(ret) )