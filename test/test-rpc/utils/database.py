import struct

str2int_map = {
    1: '<B',
    2: '<H',
    4: '<L',
    8: '<Q',
}

def str2int(s):
    global str2int_map
    return struct.unpack(str2int_map[len(s)], s)[0]

def int2str(i, size):
    global str2int_map
    return struct.pack(str2int_map[size], i)

hex_string='0123456789abcdef'
def to_string(s):
    def to_hex(i):
        global hex_string
        h,l =divmod(i, 16)
        return hex_string[h] + hex_string[l]
    t = [ to_hex(ord(i)) for i in s ]
    return ''.join(t)

STR_OFFSET_NULL = '\xff' * 8
INT_OFFSET_NULL = str2int(STR_OFFSET_NULL)

def get_payload_size(file):
    with open(file) as f:
        count = str2int( f.read(4) )
        f.seek( count * 8 + 4 + 4)
        return str2int( f.read(8) )

if __name__ == "__main__":
    print get_payload_size("/home/czp/.metaverse/mainnet/account_table")