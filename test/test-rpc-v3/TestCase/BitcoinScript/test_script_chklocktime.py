from TestCase.MVSTestCase import *
from utils import compile, set_rawtx
from binascii import b2a_hex, a2b_hex

script = '''
%(Height)s OP_CHECKLOCKTIMEVERIFY OP_DROP %(Alice)s OP_CHECKSIG
'''