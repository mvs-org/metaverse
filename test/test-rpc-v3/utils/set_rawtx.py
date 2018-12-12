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
    rawtx = '040000000154c915c45004f4779e5822899201b381341ea8d89711ef31ef3f1d4f6f9fdb790000000000ffffffff0200a3e111000000001976a9140fad171975f1100309022c83c7b9dca0a8f8e6b888ac0100000000000000f0e0ae2f0000000017a914ae9644811ebd0ad93ef8b513ac0538890e94aea387010000000000000000000000'
    sig = '304402200d10096effff0806ca51c24be8fa6c72d6f137b1497a98f70b06c46edb0d9bd80220226aeaf9fa6cf425ab609eb37fdcac82d8d18d80596953fcfba714f875d603d501'
    redeem_script = '76210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d118763ac67762102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c6484657388ac68'
    #rawtx = sys.argv[1]
    t = Transaction.parse_rawtx(rawtx)
    #t.inputs[0].utxo_hash = a2b_hex('3309eac76354785735b33ecb4238c28486de8d319231d40831859bc4c4352fa9')[::-1]
    #t.inputs[0].utxo_index = 0
    import compile
    t.inputs[0].script = a2b_hex(compile.script_to_hex(' '.join( ["OP_PUSHDATA1", sig, "02578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573", "OP_PUSHDATA1", redeem_script]))) #bytes([]) #
    #t.inputs[0].sequence = (1 << 22) | 100 # time
    #t.inputs[0].sequence =         0 | 0x0054  # height
    ret = t.to_rawtx()
    print( ret )