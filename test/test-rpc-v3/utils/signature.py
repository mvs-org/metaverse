import binascii
import hashlib
from secp256k1 import PublicKey, PrivateKey

def check_sig(public_key, signature, message):
    raw = bytes(bytearray.fromhex(public_key))
    sig = bytes(bytearray.fromhex(signature))
    pub = PublicKey(raw, raw=True)
    try:
        sig_raw = pub.ecdsa_deserialize(sig)
        good = pub.ecdsa_verify(bytes(bytearray.fromhex(message)), sig_raw)
    except:
        good = False
    print(u"{}\n".format(good))
    return 0 if good else 1

bitcoin_hash = lambda x: hashlib.sha256( hashlib.sha256(x).digest() )

def sign(priv_key, message):
    sk = PrivateKey(priv_key, True)
    pk = binascii.hexlify(sk.pubkey.serialize(True)).decode("utf-8")
    sig_raw = sk.ecdsa_sign( bytes(bytearray.fromhex(message)), digest=bitcoin_hash )
    sig = sk.ecdsa_serialize(sig_raw)
    return pk, binascii.hexlify(sig).decode("utf-8")

if __name__ == '__main__':
    Alice_priv_key = bytes(bytearray([207, 17, 143, 129, 124, 167, 244, 95, 206, 173, 55, 207, 206, 7, 180, 32, 11, 68, 128, 192, 242, 251, 52, 66, 85, 34, 107, 127, 134, 116, 249, 140]))
    Bob_priv_key = bytes(bytearray([55, 194, 110, 193, 116, 168, 94, 135, 202, 170, 222, 111, 98, 178, 201, 196, 242, 145, 127, 156, 165, 229, 66, 155, 255, 169, 220, 97, 123, 248, 32, 206]))
    message = '0400000001a92f35c4c49b853108d43192318dde8684c23842cb3eb33557785463c7ea0933000000007163522102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573210380990a7312b87abda80e5857ee6ebf798a2bf62041b07111287d19926c429d1152ae670164b3752102578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573ac68ffffffff0280f0fa02000000001976a914c96ae7acfdd70e38508760950ae308fdaf5eae1788ac010000000000000070c9fa020000000017a91419ea37b891d9a3145f7d2e581ba1afd59151af2187010000000000000000000000'
    message += '01000000'
    #import pdb; pdb.set_trace()
    pk, sig = sign(Alice_priv_key, message)
    print "Public Key:", pk
    print "Signature:", sig + '01'
    #public_key = '02578ad340083e85c739f379bbe6c6937c5da2ced52e09ac1eec43dc4c64846573'
    #assert (pk == public_key)

    #signature = '304502210094b827e0ba6f9ad0de8d140238a574ba6204ce939c32f5199d57a888f5a84ccf022068d593ce2da505c7bf34df716ddecdcfe47cd05201679490105cda55f26d826901'
    #assert (sig == signature)
    check_sig(pk, sig, message)