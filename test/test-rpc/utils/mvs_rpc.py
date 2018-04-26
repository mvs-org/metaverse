import requests
import json
import common

class RPC:
    version = "2.0"
    id = 0

    url="http://127.0.0.1:8820/rpc/v2"

    def __init__(self, method):
        self.method = method
        self.__class__.id += 1
        self.id = self.__class__.id

    def __to_data(self, positional, optional):
        optional_params = []
        for key in optional:
            value = optional[key]
            if type(value) in (tuple, list):
                for v in value:
                    optional_params.append(key)
                    optional_params.append(v)
            elif value == None:
                continue
            else:
                optional_params.append(key)
                optional_params.append(value)

        ret = {
            'method' : self.method,
            'id' : self.id,
            'jsonrpc' : self.version,
            "params": positional + optional_params}
        return json.dumps( ret )

    def post(self, positional, optional):
        rpc_rsp = requests.post(self.url, data=self.__to_data(positional, optional))
        assert (rpc_rsp.json()['id'] == self.id)
        return rpc_rsp

def handle_rpc_rsp(rpc_rsp):
    if "error" in rpc_rsp.json():
        return False, rpc_rsp.json()['error']
    return True, rpc_rsp.json()['result']

def dump_keyfile(account, password, lastword, keyfile=""):
    rpc_cmd = RPC('dumpkeyfile')
    rpc_rsp = rpc_cmd.post(
        [account, password, lastword, keyfile],
        {}
    )
    return handle_rpc_rsp(rpc_rsp)

def import_keyfile(account, password, keystore_file, keyfile_content=None):
    rpc_cmd = RPC('importkeyfile')

    if keyfile_content:
        args = [account, password, "omitted", keyfile_content]
    else:
        args = [account, password, keystore_file]

    rpc_rsp = rpc_cmd.post(
        args,
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def delete_account(account, password, lastword):
    rpc_cmd = RPC('deleteaccount')
    rpc_rsp = rpc_cmd.post(
        [account, password, lastword],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def get_account(account, password, lastword):
    rpc_cmd = RPC('getaccount')
    rpc_rsp = rpc_cmd.post(
        [account, password, lastword],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def get_publickey(account, password, address):
    rpc_cmd = RPC('getpublickey')
    rpc_rsp = rpc_cmd.post(
        [account, password, address],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def getnew_multisig(account, password, description, my_publickey, others_publickeys, required_key_num):
    assert(my_publickey not in others_publickeys)
    n = len(others_publickeys) + 1
    m = required_key_num
    rpc_cmd = RPC('getnewmultisig')
    rpc_rsp = rpc_cmd.post(
        [account, password],
        {
            '-d': description,
            '-s': my_publickey,
            '-k': others_publickeys,
            '-m' : m,
            '-n' : n
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def list_multisig(account, password):
    rpc_cmd = RPC('listmultisig')
    rpc_rsp = rpc_cmd.post(
        [account, password],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def issue_did(account, password, address, did_symbol, fee=None):
    rpc_cmd = RPC('issuedid')
    rpc_rsp = rpc_cmd.post(
        [account, password, address, did_symbol],
        {
            '--fee' : fee,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def list_dids(account=None, password=None):
    rpc_cmd = RPC('listdids')

    args = filter(None, [account, password])

    rpc_rsp = rpc_cmd.post(
        args,
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def modify_did(account, password, from_address, to_address, did_symbol):
    rpc_cmd = RPC('didmodifyaddress')
    rpc_rsp = rpc_cmd.post(
        [account, password, from_address, to_address, did_symbol],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def didsendasset(account, password, asset_symbol, amount, to_, from_=None):
    if not from_:
        cmd_args = ('didsendasset', [account, password, to_, asset_symbol, amount])
    else:
        cmd_args = ('didsendassetfrom', [account, password, from_, to_, asset_symbol, amount])
    rpc_cmd = RPC(cmd_args[0])
    rpc_rsp = rpc_cmd.post(
        cmd_args[1],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def get_asset(asset_symbol):
    rpc_cmd = RPC('getasset')
    rpc_rsp = rpc_cmd.post(
        [asset_symbol],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def get_addressasset(address, cert=None):
    rpc_cmd = RPC('getaddressasset')
    rpc_rsp = rpc_cmd.post(
        [address],
        {
            '--cert': cert,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def get_accountasset(account, password, cert=None, asset_symbol=None):
    rpc_cmd = RPC('getaccountasset')

    args = [account, password]
    if asset_symbol <> None:
        args.append(asset_symbol)

    rpc_rsp = rpc_cmd.post(
        args,
        {
            '--cert': cert,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def create_asset(account, password, symbol, volume, description=None, issuer=None, decimalnumber=None, rate=None):
    rpc_cmd = RPC('createasset')
    rpc_rsp = rpc_cmd.post(
        [account, password],
        {
            '--symbol' : symbol,
            '--volume': volume,
            '--description': description,
            '--issuer': issuer,
            '--decimalnumber': decimalnumber,
            '--rate': rate
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def issue_asset(account, password, symbol):
    rpc_cmd = RPC('issue')
    rpc_rsp = rpc_cmd.post(
        [account, password, symbol],
        {}
    )
    return handle_rpc_rsp(rpc_rsp)

def delete_localasset(account, password, symbol):
    rpc_cmd = RPC('deletelocalasset')
    rpc_rsp = rpc_cmd.post(
        [account, password],
        {
            '--symbol': symbol,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def send_asset(account, password, to_, symbol, amount, fee=None):
    cmd_args = ('sendasset', [account, password, to_, symbol, amount])

    rpc_cmd = RPC(cmd_args[0])
    rpc_rsp = rpc_cmd.post(
        cmd_args[1],
        {
            '--fee': fee,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def send_asset_from(account, password, from_, to_, symbol, amount, fee=None):
    cmd_args = ('sendassetfrom', [account, password, from_, to_, symbol, amount])

    rpc_cmd = RPC(cmd_args[0])
    rpc_rsp = rpc_cmd.post(
        cmd_args[1],
        {
            '--fee': fee,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def set_miningaccount(account, password, address):
    rpc_cmd = RPC('setminingaccount')
    rpc_rsp = rpc_cmd.post(
        [account, password, address],
        {}
    )

    return handle_rpc_rsp(rpc_rsp)

def eth_submit_work(nonce, header_hash, mix_hash):
    '''
    https://github.com/ethereum/wiki/wiki/JSON-RPC#eth_submitwork

    eth_submitWork

    Used for submitting a proof-of-work solution.
    Parameters

        DATA, 8 Bytes - The nonce found (64 bits)
        DATA, 32 Bytes - The header's pow-hash (256 bits)
        DATA, 32 Bytes - The mix digest (256 bits)

    params: [
      "0x0000000000000001",
      "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef",
      "0xD1FE5700000000000000000000000000D1FE5700000000000000000000000000"
    ]

    Returns

    Boolean - returns true if the provided solution is valid, otherwise false.
    Example

    // Request
    curl -X POST --data '{"jsonrpc":"2.0", "method":"eth_submitWork", "params":["0x0000000000000001", "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef", "0xD1GE5700000000000000000000000000D1GE5700000000000000000000000000"],"id":73}'

    // Result
    {
      "id":73,
      "jsonrpc":"2.0",
      "result": true
    }

    '''
    rpc_cmd = RPC('eth_submitWork')
    rpc_cmd.url = "http://127.0.0.1:8820/rpc/v3"
    rpc_rsp = rpc_cmd.post(
        [nonce, header_hash, mix_hash],
        {

        }
    )

    return handle_rpc_rsp(rpc_rsp)

def submit_work(nonce, header_hash, mix_hash):
    rpc_cmd = RPC('submitwork')
    rpc_rsp = rpc_cmd.post(
        [nonce, header_hash, mix_hash],
        {

        }
    )

    return handle_rpc_rsp(rpc_rsp)

def eth_get_work():
    '''
    {"jsonrpc": "2.0", "method": method, "params": params, "id": 0}
    '''
    rpc_cmd = RPC('eth_getWork')
    rpc_cmd.url = "http://127.0.0.1:8820/rpc/v3"
    rpc_rsp = rpc_cmd.post(
        [],
        {}
    )

    result, message = handle_rpc_rsp(rpc_rsp)
    assert(result)
    header_hash, seed_hash, boundary = message
    return common.toHex(header_hash), seed_hash, boundary

def get_info():
    rpc_cmd = RPC('getinfo')
    rpc_rsp = rpc_cmd.post(
        [],
        {}
    )

    result, message = handle_rpc_rsp(rpc_rsp)
    assert (result)
    height = message['height']
    difficulty = message['difficulty']
    return int(height), int(difficulty)

def create_rawtx(receivers, senders, type, deposit=None, fee=None, message=None, mychange=None, symbol=None):
    '''
    :param receivers: [address:amount, ...]
    :param senders: [address, ...]
    :param type:
        0 -- transfer etp,
        1 -- deposit etp,
        3 -- transfer asset,
        6 -- just only send message
    :param deposit: support [7, 30, 90, 182, 365] days
    :param fee:
    :param message:
    :param mychange: address
    :param symbol: asset name, not specify this option for etp tx
    :return:
    '''
    rpc_cmd = RPC('createrawtx')
    rpc_rsp = rpc_cmd.post(
        [],
        {
            '--receivers': receivers,
            '--senders': senders,
            '--type': type,
            '--deposit': deposit,
            '--fee': fee,
            '--message': message,
            '--mychange': mychange,
            '--symbol': symbol,
        }
    )

    result, message = handle_rpc_rsp(rpc_rsp)
    if result:
        return result, message["hex"]
    return result, message

def send(account, password, to_, amount, fee=None, desc=None):
    rpc_cmd = RPC('send')
    rpc_rsp = rpc_cmd.post(
        [account, password, to_, amount],
        {
            '-f': fee,
            '-m': desc,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def sendfrom(account, password, from_, to_, amount, fee=None, desc=None):
    rpc_cmd = RPC('sendfrom')
    rpc_rsp = rpc_cmd.post(
        [account, password, from_, to_, amount],
        {
            '-f': fee,
            '-m': desc,
        }
    )

    return handle_rpc_rsp(rpc_rsp)

def didsendfrom(account, password, amount, to_, from_, fee=None, desc=None):
    cmd_args = ('didsendfrom', [account, password, from_, to_, amount])
    rpc_cmd = RPC(cmd_args[0])
    rpc_rsp = rpc_cmd.post(
        cmd_args[1],
        {
            '-f': fee,
            '-m': desc,
        }
    )
    return handle_rpc_rsp(rpc_rsp)

def didsend(account, password, amount, to_, fee=None, desc=None):
    rpc_cmd = RPC( 'didsend' )
    rpc_rsp = rpc_cmd.post(
        [account, password, to_, amount],
        {
            '-f': fee,
            '-m': desc,
        }
    )
    return handle_rpc_rsp(rpc_rsp)

