import os, time

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
    return time.strftime('%Y%m%d.%H%M%S', time.localtime(time.time()))