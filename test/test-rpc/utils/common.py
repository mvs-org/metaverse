import os
from datetime import datetime

def remove_file(file_path):
    if os.path.exists(file_path):
        os.remove(file_path)

def toHex(s):
    if s[:2] == '0x':
        s = s[2:]
    return ''.join(   [chr(int(s[i:i + 2], 16)) for i in xrange(0, len(s), 2)]   )

def toString(h):
    return ''.join(['%02x' % ord(i) for i in h])

def get_timestamp():
    now = datetime.now()
    return now.strftime("%Y%m%dT%H%M%ST%f")

def create_multisig_address(roles, required_key_num):
    assert( required_key_num <= len(roles) )
    desc = ' & '.join([i.name for i in roles]) + '\'s Multisig Address'
    for i, role in enumerate(roles):
        addr = role.new_multisigaddress(desc, roles[:i] + roles[i+1:], required_key_num)
    return addr
