import requests
import json

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

def import_keyfile(account, password, keystore_file):
    rpc_cmd = RPC('importkeyfile')
    rpc_rsp = rpc_cmd.post(
        [account, password, keystore_file],
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

if __name__ == '__main__':
    #dumpkeyfile('czp', '123456', 'broken')
    print importkeyfile('mvs_test', 'tkggfngu', os.path.abspath( '../resource/keystore/mvs_tkggfngu.json') )