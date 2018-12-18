#!/usr/bin/python
# -*- coding: utf-8 -*-
from Crypto.Hash import MD5

from Crypto.Cipher import AES
from Crypto import Random
import base64

def toString(s):
    return ''.join(['%02x' % ord(i) for i in s])

def toBase64(s):
    return base64.b64encode(s)

def derive_key(passphrase, salt):
    key_size = 32
    iv_size = 16
    iterations = 1

    derivedKeyWords = b''
    block = b''

    while len(derivedKeyWords) < (key_size + iv_size):
        hash = MD5.new()
        hash.update(block + passphrase + salt)

        block = hash.digest()
        derivedKeyWords += block
    key = derivedKeyWords[:key_size]
    iv = derivedKeyWords[key_size:]
    return key, iv


def Pkcs7(data, block_size=16):
    nPaddingBytes = block_size - (len(data) % block_size)
    return chr(nPaddingBytes) * nPaddingBytes


def AES_CBC_encrypt(message, key, iv):
    cipher = AES.new(key, AES.MODE_CBC, iv)
    msg = cipher.encrypt(message + Pkcs7(message))
    return msg


def AES_CBC_decrypt(cipher_txt, passphrase):
    passphrase = bytes(passphrase, 'utf-8')
    s = base64.b64decode(cipher_txt)
    salt = s[8:16]
    raw_cipher_txt = s[16:]
    key, iv = derive_key(passphrase, salt)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    msg = cipher.decrypt(raw_cipher_txt)
    padding = msg[-1]
    return str(msg[:-padding], 'utf-8')


