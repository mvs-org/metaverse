import compile

script = '''
OP_DUP OP_HASH160 <Alice> OP_EQUAL OP_IF
  OP_CHECKSIG
OP_ELSE
  OP_DUP OP_HASH160 <Bob> OP_EQUALVERIFY OP_CHECKSIGVERIDY bfffffff OP_CHECKSEQUENCEVERIFY
OP_ENDIF
'''
print( compile.script_to_hex(script) )