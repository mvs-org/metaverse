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

def mvs_api(func):
    def wrapper(*args, **kwargs):
        cmd, positional, optional, result_handler = func(*args, **kwargs)
        rpc_cmd = RPC(cmd)
        rpc_rsp = rpc_cmd.post(positional, optional)
        if "error" in rpc_rsp.json():
            return rpc_rsp.json()['error']['code'], rpc_rsp.json()['error']['message']
        if result_handler:
            return 0, result_handler(rpc_rsp.json()['result'])
        return 0, rpc_rsp.json()['result']
    return wrapper

def mvs_api_v3(func):
    def wrapper(*args, **kwargs):
        cmd, positional, optional, result_handler = func(*args, **kwargs)
        rpc_cmd = RPC(cmd)
        rpc_cmd.url = RPC.url.replace('/v2', '/v3')
        rpc_rsp = rpc_cmd.post(positional, optional)
        if "error" in rpc_rsp.json():
            return rpc_rsp.json()['error']['code'], rpc_rsp.json()['error']['message']
        if result_handler:
            return 0, result_handler(rpc_rsp.json()['result'])
        return 0, rpc_rsp.json()['result']
    return wrapper

@mvs_api
def getpeerinfo():
    '''
    {
        "peers" :  <-- return
        [
            "10.10.10.184:52644"
        ]
    }
    '''
    return "getpeerinfo", [], {}, lambda result: result["peers"]

@mvs_api
def dump_keyfile(account, password, lastword, keyfile=""):
    return "dumpkeyfile", [account, password, lastword, keyfile], {}, None

@mvs_api
def import_keyfile(account, password, keystore_file, keyfile_content=None):
    if keyfile_content:
        args = [account, password, "omitted", keyfile_content]
    else:
        args = [account, password, keystore_file]
    return "importkeyfile", args, {}, None

@mvs_api
def delete_account(account, password, lastword):
    return "deleteaccount", [account, password, lastword], {}, None

@mvs_api
def get_account(account, password, lastword):
    return "getaccount", [account, password, lastword], {}, None

@mvs_api
def get_publickey(account, password, address):
    return "getpublickey", [account, password, address], {}, lambda result: result["public-key"]

@mvs_api
def getnew_multisig(account, password, description, my_publickey, others_publickeys, required_key_num):
    assert(my_publickey not in others_publickeys)
    n = len(others_publickeys) + 1
    m = required_key_num

    return "getnewmultisig", \
           [account, password], \
           {
            '-d': description,
            '-s': my_publickey,
            '-k': others_publickeys,
            '-m' : m,
            '-n' : n
           }, \
           None

@mvs_api
def list_multisig(account, password):
    return "listmultisig", [account, password], {}, None

@mvs_api
def issue_did(account, password, address, did_symbol, fee=None):
    return "issuedid", [account, password, address, did_symbol], {'--fee' : fee}, None

@mvs_api
def list_dids(account=None, password=None):
    return "listdids", [account, password], {}, None

@mvs_api
def modify_did(account, password, from_address, to_address, did_symbol):
    return "didmodifyaddress", [account, password, from_address, to_address, did_symbol], {}, None

@mvs_api
def get_asset(asset_symbol=None):
    return "getasset", filter(None, [asset_symbol]), {}, None

@mvs_api
def get_addressasset(address, cert=None):
    return "getaddressasset", [address], {'--cert': cert}, None

@mvs_api
def get_accountasset(account, password, cert=None, asset_symbol=None):
    rpc_cmd = RPC('getaccountasset')

    args = [account, password]
    if asset_symbol <> None:
        args.append(asset_symbol)

    return "getaccountasset", args, {'--cert': cert}, None

@mvs_api
def create_asset(account, password, symbol, volume, description=None, issuer=None, decimalnumber=None, rate=None):
    optionals = {
        '--symbol' : symbol,
        '--volume': volume,
        '--description': description,
        '--issuer': issuer,
        '--decimalnumber': decimalnumber,
        '--rate': rate
    }

    return "createasset", [account, password], optionals, None

@mvs_api
def issue_asset(account, password, symbol):
    return "issue", [account, password, symbol], {}, None

@mvs_api
def delete_localasset(account, password, symbol):
    return "deletelocalasset", [account, password], {'--symbol': symbol}, None

@mvs_api
def send_asset(account, password, to_, symbol, amount, fee=None):
    return "sendasset", [account, password, to_, symbol, amount], {'--fee': fee}, None

@mvs_api
def send_asset_from(account, password, from_, to_, symbol, amount, fee=None):
    return "sendassetfrom", [account, password, from_, to_, symbol, amount], {'--fee': fee}, None

@mvs_api
def set_miningaccount(account, password, address):
    return "setminingaccount", [account, password, address], {}, None

@mvs_api_v3
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
    return "eth_submitWork", [nonce, header_hash, mix_hash], {}, None

@mvs_api
def submit_work(nonce, header_hash, mix_hash):
    return "submitwork", [nonce, header_hash, mix_hash], {}, None

@mvs_api_v3
def eth_get_work():
    '''
    {"jsonrpc": "2.0", "method": method, "params": params, "id": 0}
    '''
    def result_handler(result):
        header_hash, seed_hash, boundary = result
        return common.toHex(header_hash), seed_hash, boundary

    return "eth_getWork", [], {}, result_handler

@mvs_api
def get_info():
    return "getinfo", [], {}, lambda result: (int(result['height']), int(result['difficulty']))

@mvs_api
def send(account, password, to_, amount, fee=None, desc=None):
    positional = [account, password, to_, amount],
    optional =   {
            '-f': fee,
            '-m': desc,
        }

    return "send", positional, optional, None

@mvs_api
def sendfrom(account, password, from_, to_, amount, fee=None, desc=None):
    positional = [account, password, from_, to_, amount],
    optional = {
        '-f': fee,
        '-m': desc,
    }

    return "sendfrom", positional, optional, None

@mvs_api
def didsendfrom(account, password, amount, to_, from_, fee=None, desc=None):
    return "didsendfrom", [account, password, from_, to_, amount], {'-f': fee, '-m': desc}, None

@mvs_api
def didsend(account, password, amount, to_, fee=None, desc=None):
    return "didsend", [account, password, to_, amount], {'-f': fee, '-m': desc}, None

