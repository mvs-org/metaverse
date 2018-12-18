from Crypto.Cipher import AES
import base64
import hashlib

def ripemd160_hash(data):
    obj = hashlib.new('ripemd160', data)
    return obj.digest(), obj.hexdigest()

def sha256_hash(data):
    return hashlib.sha256(data).digest()

import struct
import zlib
import os, sys
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
char2i = {}
for i, char in enumerate(hex_string):
    char2i[char] = i

def to_string(s):
    def to_hex(i):
        global hex_string
        h,l =divmod(i, 16)
        return hex_string[h] + hex_string[l]
    t = [ to_hex(ord(i)) for i in s ]
    return ''.join(t)


def to_bin(ss):
    global char2i
    bin_lst = [chr(char2i[ss[i]] * 16 + char2i[ss[i + 1]]) for i in range(0, len(ss), 2)]
    bin_lst.reverse()
    return ''.join( bin_lst )



def get_var_len(ff, sum):
    a = ord( ff.read(1) )
    if a < 0xfd:
        sum(8)
        return a
    elif a == 0xfd:
        sum(6)
        return str2int(ff.read(2))
    elif a == 0xfe:
        sum(4)
        return str2int(ff.read(4))
    else:
        return str2int(ff.read(8))

class Header:
    def __init__(self, size_of_offset, extra_bytes):
        # read from database file
        self.bucket_size = 0
        self.offset_begin = 0

        # pre-define by each table
        self.size_of_offset = size_of_offset
        self.payload_size = 0
        self.extra_bytes = extra_bytes

    def parse_header(self, f, need_bucket=True):
        f.seek(0)

        self.bucket_size = str2int( f.read(4) )
        self.offset_begin = 4 + self.bucket_size * self.size_of_offset + self.extra_bytes

        invalid_offset = '\xFF' * self.size_of_offset
        bucket_array = []
        if need_bucket:
            for i in range(self.bucket_size):
                offset_str = f.read(self.size_of_offset)
                if offset_str == invalid_offset:
                    continue
                bucket_array.append( (i, str2int( offset_str )) )
        else:
            f.seek(f.tell() + self.bucket_size*self.size_of_offset)

        if self.extra_bytes:
            f.read( self.extra_bytes )
        self.payload_size = str2int( f.read( self.size_of_offset ) )
        return bucket_array

class Slab:
    def __init__(self, size_of_key, size_of_next, parse_value_func):
        self.size_of_key  = size_of_key
        self.size_of_next = size_of_next
        self.parse_value_func = parse_value_func

class Record:
    def __init__(self, size_of_key, size_of_next, size_of_value):
        self.size_of_key = size_of_key  # uint160_t, ripemd160_hash of address
        self.size_of_next = size_of_next  # uint32_t, offset from payload_size
        self.size_of_value = size_of_value # uint32_t, index of array
        self.record_size = size_of_key + size_of_next + size_of_value  # key, next, value

class HeadManager:
    def __init__(self, fw, size_of_offset):
        self.fw = fw
        self.current_slot = 0
        self.size_of_offset = size_of_offset

        self.STR_OFFSET_NULL = b'\xFF' * size_of_offset
        self.INT_OFFSET_NULL = str2int( self.STR_OFFSET_NULL )

    def append_slot(self, index, offset):
        while self.current_slot < index:
            self.current_slot += 1
            self.fw.write(self.STR_OFFSET_NULL)
        if self.current_slot != index:
            return
        self.current_slot += 1
        self.fw.write(int2str(offset, self.size_of_offset))

class RowsManager:
    def __init__(self, record_size, fr, fw):
        self.record_size = record_size
        self.fr =fr
        self.fw = fw
        self.rows = 0
        self.fr.seek(0)
        self.max_record_count = str2int(self.fr.read(4))

    def append_row(self, read_index):
        next = read_index
        while next != 0xFFFFFFFF:
            assert (next < self.max_record_count)


            self.fr.seek( 4 + self.record_size * next )
            next_ = str2int( self.fr.read(4) )
            value = self.fr.read(self.record_size-4)

            if next == next_: # self recursive
                next = 0xFFFFFFFF
            else:
                next = next_

            self.rows += 1
            if next == 0xFFFFFFFF:
                self.fw.write(int2str(0xFFFFFFFF, 4))
            else:
                self.fw.write(int2str(self.rows, 4))
            self.fw.write(value)

class account_table:
    def __init__(self):
        self.filename = 'account_table'
        self.header = Header(8, 4)
        self.slab = Slab(32, 8, self.parse_value_func)

    def re_arrange(self, table_dir):
        fw_head = open("./%s_head" % self.filename, 'wb')
        fw_body = open("./%s_body" % self.filename, 'wb')
        fr_table= open(table_dir + '/' + self.filename, 'rb')

        hm = HeadManager(fw_head, self.header.size_of_offset)

        account_info = self.header.parse_header(fr_table)

        fw_head.write(int2str(self.header.bucket_size, 4))

        payload_size = self.header.size_of_offset
        for index, offset in account_info:
            hm.append_slot(index, payload_size)

            next = offset

            while next != hm.INT_OFFSET_NULL:
                fr_table.seek(self.header.offset_begin + next)
                key = fr_table.read(self.slab.size_of_key)
                next = str2int( fr_table.read(self.slab.size_of_next) )
                begin, end = self.slab.parse_value_func(fr_table)

                payload_size += self.slab.size_of_key + self.slab.size_of_next + (end - begin)

                fw_body.write(key)
                if next != hm.INT_OFFSET_NULL:
                    fw_body.write(int2str(payload_size, self.header.size_of_offset))
                else:
                    fw_body.write(hm.STR_OFFSET_NULL)



                fr_table.seek( begin )
                fw_body.write( fr_table.read( end - begin ) )
        hm.append_slot(self.header.bucket_size - 1, hm.INT_OFFSET_NULL)
        fw_head.write(b'\x00' * self.header.extra_bytes)
        fw_head.write( int2str(payload_size, self.header.size_of_offset) )

        fr_table.close()
        fw_head.close()
        fw_body.close()

    @classmethod
    def parse_value_func(cls, ff):
        begin = ff.tell()

        extra_padding = []
        account_name_len = get_var_len(ff, extra_padding.append)
        assert (0 < account_name_len <= 64)
        account_name = ff.read(account_name_len)
        mnemonic_len = get_var_len(ff, extra_padding.append)
        if mnemonic_len != 0:
            assert (mnemonic_len % 16 == 1)
        # print "mnemonic_len:", mnemonic_len
        mnemonic = ff.read(mnemonic_len)
        # print "mnemonic", to_string(mnemonic)
        passwd_hash = ff.read(32)
        hd_index = str2int(ff.read(4))
        # print 'hd_index:', hd_index
        priority = ff.read(1)
        type = ff.read(1)
        status = ff.read(1)

        if type == 1:  # multisig account
            vec_size = str2int(ff.read(4))
            #print '\tvec_size', vec_size
            for i in range(vec_size):
                hd = str2int(ff.read(4))
                id = str2int(ff.read(4))
                m = ord(ff.read(1))
                n = ord(ff.read(1))
                l = get_var_len(ff, extra_padding.append)
                pubkey = ff.read(l)
                size_ = ord(ff.read(1))
                for j in range(size_):
                    l = get_var_len(ff, extra_padding.append)
                    cosigner_pubkey = ff.read(l)
                desc = ff.read(get_var_len(ff, extra_padding.append))
                address = ff.read(get_var_len(ff, extra_padding.append))

        end = ff.tell() + sum(extra_padding)
        if account_name == bytes(sys.argv[2], "utf8") :#and passwd_hash == sys.argv[3]:
            secret = bytes(sys.argv[3], 'utf8')
            sec = sha256_hash(ripemd160_hash(secret)[0])
            cipher = AES.new(sec, AES.MODE_ECB)
            print( mnemonic[1:])
            print(cipher.decrypt(mnemonic[1:]).rstrip(b'\x00'))

        return begin, end

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print('Usage: python3.6 dump_key.py "mvs-wallet-dir" "account-name" "password"')
        print('   eg: python3.6 dump_key.py "/home/czp/.metaverse/mainnet" "test" "123456"')
        exit(0)
    acc = account_table()
    acc.re_arrange(sys.argv[1])
