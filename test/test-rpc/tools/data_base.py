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
            for i in xrange(self.bucket_size):
                offset_str = f.read(self.size_of_offset)
                if offset_str == invalid_offset:
                    continue
                bucket_array.append( (i, str2int( offset_str )) )
        else:
            f.seek(f.tell() + self.bucket_size*self.size_of_offset)

        if self.extra_bytes:
            to_string(f.read( self.extra_bytes ))
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

        self.STR_OFFSET_NULL = '\xFF' * size_of_offset
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
        while next <> 0xFFFFFFFF:
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
                if next <> hm.INT_OFFSET_NULL:
                    fw_body.write(int2str(payload_size, self.header.size_of_offset))
                else:
                    fw_body.write(hm.STR_OFFSET_NULL)



                fr_table.seek( begin )
                fw_body.write( fr_table.read( end - begin ) )
        hm.append_slot(self.header.bucket_size - 1, hm.INT_OFFSET_NULL)
        fw_head.write('\x00' * self.header.extra_bytes)
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
        if mnemonic_len <> 0:
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

        return begin, end

    def query(self, key, table_dir):
        def std_hash(any):
            seed = 0
            for i in any:
                i = ord(i)
                seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2)
                seed &= 0xFFFFFFFFFFFFFFFF
            return seed
        #print std_hash(hash_)
        key = to_bin(key)

        with open(table_dir + '/' + self.filename, 'rb') as fr_table:
            self.header.parse_header(fr_table, False)

            index = std_hash(key) % self.header.bucket_size

            fr_table.seek( 4 + index * self.header.size_of_offset )
            offset = fr_table.read( self.header.size_of_offset )
            while offset <> '\xFF' * self.header.size_of_offset:
                offset = str2int(offset)
                assert (offset < self.header.payload_size)

                fr_table.seek(self.header.offset_begin + offset)
                key_, next_ = fr_table.read(self.slab.size_of_key), fr_table.read(self.slab.size_of_next)
                if key_ == key:
                    begin, end = self.parse_value_func(fr_table)
                    fr_table.seek(begin)
                    return fr_table.read(end - begin)
                offset = next_


class asset_table(account_table):
    def __init__(self):
        self.filename = 'asset_table'
        self.header = Header(8, 4)
        self.slab = Slab(32, 8, self.parse_value_func)

    @classmethod
    def parse_value_func(cls, ff):
        begin = ff.tell()
        version_ = str2int( ff.read(4) )
        hash_ = ff.read(32)
        index_ = str2int( ff.read(4) )
        height_ = str2int( ff.read(8) )

        extra_padding = []
        symbol_len = get_var_len(ff, extra_padding.append)
        assert (0 < symbol_len <= 64)

        symbol_name = ff.read(symbol_len)
        maximum_supply = str2int(ff.read(8))
        decimal_number = str2int(ff.read(1))
        secondaryissue_threshold = str2int(ff.read(1))
        unused2 = str2int(ff.read(1))
        unused3 = str2int(ff.read(1))

        assert (unused2 == unused3 == 0)

        issuer_len = get_var_len(ff, extra_padding.append)
        assert (0 < issuer_len <= 64)
        issuer_name = ff.read(issuer_len)

        address_len = get_var_len(ff, extra_padding.append)
        assert (0 < address_len <= 64)
        address = ff.read(address_len)

        description_len = get_var_len(ff, extra_padding.append)
        assert (0 <= description_len <= 100) # optional
        description = ff.read(description_len)

        end = ff.tell()
        return begin, end

class cert_table(account_table):
    def __init__(self):
        self.filename = 'cert_table'
        self.header = Header(8, 4)
        self.slab = Slab(32, 8, self.parse_value_func)

    @classmethod
    def parse_value_func(cls, ff):
        begin = ff.tell()

        extra_padding = []

        symbol_len = get_var_len(ff, extra_padding.append)
        assert (0 < symbol_len <= 64)
        symbol_ = ff.read(symbol_len)

        owner_len = get_var_len(ff, extra_padding.append)
        assert (0 < owner_len <= 64)
        owner_ = ff.read(owner_len)

        address_len = get_var_len(ff, extra_padding.append)
        assert (0 < address_len <= 64)
        address_ = ff.read(address_len)

        cert_type_ = str2int( ff.read(4) )
        assert (0 <= cert_type_ <= 3)
        status_ = str2int( ff.read(1) )
        assert (0 <= status_ <= 3)

        end = ff.tell()
        return begin, end

class did_table(account_table):
    def __init__(self):
        self.filename = 'did_table'
        self.header = Header(8, 4)
        self.slab = Slab(32, 8, self.parse_value_func)

    @classmethod
    def parse_value_func(cls, ff):
        begin = ff.tell()

        version_ = str2int(ff.read(4))
        hash_ = ff.read(32)
        index_ = str2int(ff.read(4))
        height_ = str2int(ff.read(8))
        status_ = str2int(ff.read(4))
        assert (0 < status_ <= 2)

        symbol_len = str2int(ff.read(1))
        assert (0 < symbol_len <= 64)
        symbol = ff.read(symbol_len)
        address_len = str2int(ff.read(1))
        assert (0 < address_len <= 64)
        address = ff.read(address_len)

        end = ff.tell()
        return begin, end

class transaction_table(account_table):
    def __init__(self):
        self.filename = 'transaction_table'
        self.header = Header(8, 4)
        self.slab = Slab(32, 8, self.parse_value_func)

    @classmethod
    def parse_etp(cls, ff):
        pass

    @classmethod
    def parse_etp_award(cls, ff):
        height = ff.read(8)

    @classmethod
    def parse_asset(cls, ff):
        extra_padding = []
        def asset_detail(ff):
            var_len = get_var_len(ff, extra_padding.append)
            assert (0 <= var_len <= 64)
            symbol = ff.read(var_len)
            maximum_supply = ff.read(8)
            decimal_number = ff.read(1)
            secondaryissue_threshold = ff.read(1)
            unused2 = str2int( ff.read(1) )
            unused3 = str2int( ff.read(1) )

            assert (0 == unused2 == unused3)

            var_len = get_var_len(ff, extra_padding.append)
            assert (0 <= var_len <= 64)
            issuer = ff.read(var_len)
            var_len = get_var_len(ff, extra_padding.append)
            assert (0 <= var_len <= 64)
            address = ff.read(var_len)
            var_len = get_var_len(ff, extra_padding.append)
            assert (0 <= var_len <= 100)
            description = ff.read(var_len)

        def asset_transfer(ff):
            var_len = get_var_len(ff, extra_padding.append)
            assert (0 <= var_len <= 64)
            symbol = ff.read(var_len)

            quantity = ff.read(8)
        status = str2int( ff.read(4) )
        if status == 1:
            asset_detail(ff)
        elif status == 2:
            asset_transfer(ff)
        else:
            assert (False)

    @classmethod
    def parse_message(cls, ff):
        extra_padding = []
        var_len = get_var_len(ff, extra_padding.append)
        assert (0 <= var_len <= 300)
        message = ff.read(var_len)

    @classmethod
    def parse_did(cls, ff):
        status = str2int( ff.read(4) )
        assert (0 < status <= 2)
        extra_padding = []
        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        symbol = ff.read(var_len)

        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        address = ff.read(var_len)



    @classmethod
    def parse_asset_cert(cls, ff):
        extra_padding = []
        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        symbol = ff.read(var_len)

        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        owner = ff.read(var_len)

        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        address = ff.read(var_len)

        cert_type = str2int( ff.read(4) )
        assert (0 <= cert_type <= 3)

        status = str2int( ff.read(1) )
        assert (0 <= status <= 3)

    @classmethod
    def parse_asset_mit(cls, ff):
        status = str2int( ff.read(1) )

        extra_padding = []
        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        symbol = ff.read(var_len)

        var_len = get_var_len(ff, extra_padding.append)
        assert (0 < var_len <= 64)
        address = ff.read(var_len)

        assert (0 < (status % 128) <= 2 )

        if status == 1:
            var_len = get_var_len(ff, extra_padding.append)
            assert (0 <= var_len <= 256)
            content = ff.read(var_len)



    @classmethod
    def parse_value_func(cls, ff):
        begin = ff.tell()

        attachment_parser = {
            0: cls.parse_etp,
            1: cls.parse_etp_award,
            2: cls.parse_asset,
            3: cls.parse_message,
            4: cls.parse_did,
            5: cls.parse_asset_cert,
            6: cls.parse_asset_mit,
        }

        hight = str2int( ff.read(4) )
        index = str2int( ff.read(4) )
        assert (index < 1000)

        # transaction
        extra_padding = []
        version = str2int( ff.read(4) )
        assert (version < 5)
        input_size = get_var_len(ff, extra_padding.append)
        assert (input_size < 700) # max input ~ 667 ?
        for i in xrange(input_size):
            #previous_output
            utxo_hash_ = ff.read(32)
            utxo_index = str2int( ff.read(4) )

            #scprit
            script_len = get_var_len(ff, extra_padding.append)
            assert (script_len < 1000)
            script = ff.read(script_len)
            #seq
            sequence = str2int( ff.read(4) )

        output_size = get_var_len(ff, extra_padding.append)
        assert (output_size < 70)
        for i in xrange(output_size):
            amount = str2int( ff.read(8) )
            script_len = get_var_len(ff, extra_padding.append)
            assert (script_len < 256)
            script = ff.read(script_len)

            #attach data
            attach_version = str2int( ff.read(4) )
            attach_type = str2int( ff.read(4) )
            if attach_version == 207:
                var_len = get_var_len(ff, extra_padding.append)
                assert (0 <= var_len <= 64)
                todid = ff.read(var_len)

                var_len = get_var_len(ff, extra_padding.append)
                assert (0 <= var_len <= 64)
                fromdid = ff.read(var_len)
            # assert attach_type in attachment_parser
            attachment_parser[attach_type](ff)

        locktime = str2int( ff.read(4) )
        end = ff.tell()
        return begin, end

class address_did_table:
    def __init__(self):
        self.table_filename = 'address_did_table'
        self.row_filename = 'address_did_row'

        self.size_of_record = 219 # record row size

        self.header = Header(4, 0)
        self.record = Record(20, 4, 4)

    def re_arrange(self, table_dir):
        fw_head = open("./%s_head" % self.table_filename, 'wb')
        fw_body = open("./%s_body" % self.table_filename, 'wb')
        fw_rows = open("./%s_rows" % self.table_filename, 'wb')
        fr_table= open(table_dir + '/' + self.table_filename, 'rb')
        fr_rows = open(table_dir + '/' + self.row_filename, 'rb')

        rm = RowsManager(self.size_of_record, fr_rows, fw_rows)
        hm = HeadManager(fw_head, self.header.size_of_offset)

        address_did_info = self.header.parse_header(fr_table)
        # record's offset begin pos is different from slab
        self.header.offset_begin += self.header.size_of_offset

        fw_head.write(int2str(self.header.bucket_size, 4))

        #print 'bucket_size:', self.header.bucket_size
        #print 'payload_size:', self.header.payload_size
        #print 'offset_begin:', self.header.offset_begin

        payload_size = 0
        for index, offset in address_did_info:
            hm.append_slot(index, payload_size)

            next = offset

            buffer = []
            while next != hm.INT_OFFSET_NULL:
                assert (next < self.header.payload_size)

                fr_table.seek(self.header.offset_begin + next * self.record.record_size)
                key = fr_table.read(self.record.size_of_key)
                next = str2int( fr_table.read(self.record.size_of_next) )

                value=  fr_table.read(self.record.size_of_value)

                buffer.append( (key, next, value) )

            buffer.sort(key = lambda x: x[0])

            for key, next, value in buffer:
                payload_size += 1

                fw_body.write(key)
                if next <> hm.INT_OFFSET_NULL:
                    fw_body.write(int2str(payload_size, self.header.size_of_offset))
                else:
                    fw_body.write(hm.STR_OFFSET_NULL)

                fw_body.write( int2str(rm.rows, self.record.size_of_value) )
                rm.append_row(str2int(value))
        print 'total rows:', rm.rows

        hm.append_slot(self.header.bucket_size - 1, hm.INT_OFFSET_NULL)
        fw_head.write('\x00' * self.header.extra_bytes)
        fw_head.write( int2str(payload_size, self.header.size_of_offset) )

        fr_table.close()
        fr_rows.close()
        fw_head.close()
        fw_body.close()
        fw_rows.close()

class account_address_table(address_did_table):
    def __init__(self):
        self.table_filename = 'account_address_table'
        self.row_filename = 'account_address_rows'

        self.size_of_record = 365  # record row size

        self.header = Header(4, 0)
        self.record = Record(20, 4, 4)

class address_asset_table(address_did_table):
    def __init__(self):
        self.table_filename = 'address_asset_table'
        self.row_filename = 'address_asset_row'

        self.size_of_record = 359  # record row size

        self.header = Header(4, 0)
        self.record = Record(20, 4, 4)

class history_table(address_did_table):
    def __init__(self):
        self.table_filename = 'history_table'
        self.row_filename = 'history_rows'

        self.size_of_record = 85  # record row size

        self.header = Header(4, 0)
        self.record = Record(20, 4, 4)


all_tables = [account_table, asset_table, cert_table, did_table, transaction_table,
              address_did_table, account_address_table, address_asset_table, history_table]

def GetFileMd5(filename):
    import hashlib
    if not os.path.isfile(filename):
        return
    myhash = hashlib.md5()
    f = file(filename,'rb')
    while True:
        b = f.read(8096)
        if not b :
            break
        myhash.update(b)
    f.close()
    return myhash.hexdigest()

def crc32(filepath):
    block_size = 1024 * 1024
    crc = 0

    try:
        fd = open(filepath, 'rb')
        while True:
            buffer = fd.read(block_size)
            if len(buffer) == 0: # EOF or file empty. return hashes
                fd.close()
                if sys.version_info[0] < 3 and crc < 0:
                    crc += 2 ** 32
                return crc
            crc = zlib.crc32(buffer, crc)
    except Exception as e:
        if sys.version_info[0] < 3:
            error = unicode(e)
        else:
            error = str(e)
        return 0, error

if __name__ == "__main__":
    import getpass
    user = getpass.getuser()
    mainnet_dir = '/home/%s/.metaverse/mainnet/' % user
    #at = transaction_table()
    #at.re_arrange('/home/czp/.metaverse/mainnet/')
    #import pdb;pdb.set_trace()
    #data = at.query("422e2ea8ac1d333b61672044ad7e83b384c7ee621632fbe83dca0510278f7616", mainnet_dir)
    #print to_string(data)

    for table in all_tables: #[address_asset_table]:
        t = table()
        print "begin to re_arrage: ", t.__class__
        t.re_arrange(mainnet_dir)

    ret = {}
    for i in os.listdir('.'):
        if i[-5:] in ("_head", "_body", "_rows"):
            ret[i] = GetFileMd5(i)
    print ret