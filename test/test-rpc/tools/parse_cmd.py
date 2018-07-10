import os, sys
import re

cmd_dir = '/home/czp/GitHub/nova/metaverse/include/metaverse/explorer/extensions/commands'

rettypes = '''
getblockheader, blockheader
getblock, block
getnewaccount,account
getnewaddress,List<String>
listaddresses,List<String>
validateaddress,address
importaccount,accountEx
importkeyfile,String
dumpkeyfile,String
changepasswd,accountstatus
deleteaccount,accountstatus
getaccount,accountresult
shutdown,String
getinfo,info
getheight,UInt64
getpeerinfo,List<String>
getmininginfo,mining_info
startmining,String
stopmining,String
getwork,List<String>
addnode,String
submitwork,Boolean
getmemorypool,List<transaction>
getbalance,balances
listbalances,List<balance>
deposit,transaction
send,transaction
sendfrom,transaction
sendmore,transaction
gettx,transaction
listtxs,transactions
createasset,asset
deletelocalasset,delasset
issue,transaction
getaccountasset,List<asset>
getaddressasset,List<asset>
getasset,List<String>
listassets,List<asset>
sendasset,transaction
sendassetfrom,transaction
secondaryissue,transaction
issuecert,transaction
transfercert,transaction
burn,transaction
getnewmultisig,mulsignature
listmultisig,List<mulsignature>
deletemultisig,mulsignature
createmultisigtx,String
signmultisigtx,String
createrawtx,String
signrawtx,rawtx
decoderawtx,transaction
sendrawtx,String
registerdid,transaction
listdids,List<did>
didsend,transaction
didsendmore,transaction
didsendfrom,transaction
didsendasset,transaction
didsendassetfrom,transaction
didchangeaddress,transaction
getdid,List<didhistoryaddress>
registermit,transaction
transfermit,transaction
listmits,List<mit>
getmit,List<mit>
getpublickey,String
popblock,String
fetchheaderext,String
getaddressetp,String
setminingaccount,String
'''

func2type = {}
for i in rettypes.strip().split('\n'):
    k,v = i.split(',')
    func2type[k.strip()] = v.strip()

def get_paraname(line):
    pattern = '"([/\w]+)(,\w+){0,1}"'
    m = re.search(pattern, line)
    if m:
        ret = m.groups()[0]
    else:
        ret = line.strip()
    return ret

def get_paradef(line):
    pattern = "value\\<([\s\S]+)\\>"
    ret = {}
    paradef = line.split('->')
    m = re.search(pattern, paradef[0])
    ret["para_type"] = m.groups()[0]
    for i in paradef[1:]:
        if 'required' in i:
            ret["must_give"] = True
        elif 'default_value' in i:
            pattern = 'default_value\\(([\s\S]+)\\)'

            m = re.search(pattern, i)
            ret["default_value"] = m.groups()[0].strip()
        elif 'zero_tokens' in i:
            ret["just_token"] = True
        else:
            print i
            assert False
    return ret

def macro_expand(input):
    convert = {
        'BX_ACCOUNT_NAME' : "Account name required.",
        'BX_ACCOUNT_AUTH' : "Account password(authorization) required.",
        'BX_MST_OFFERING_CURVE' : '''The token offering model by block height. 
    TYPE=1 - fixed quantity model; TYPE=2 - specify parameters; 
    LQ - Locked Quantity each period; 
    LP - Locked Period, numeber of how many blocks; 
    UN - Unlock Number, number of how many LPs; 
    eg: 
        TYPE=1;LQ=9000;LP=60000;UN=3  
        TYPE=2;LQ=9000;LP=60000;UN=3;UC=20000,20000,20000;UQ=3000,3000,3000 
    defaults to disable.''',
        'BX_ADMIN_NAME' : "Administrator required.(when administrator_required in mvs.conf is set true)",
        'BX_ADMIN_AUTH' : "Administrator password required.",
    }

    return convert.get(input, input)

special_cmds = []

def parse_cmd(filename):
    starts = 'BX_HELP_VARIABLE ",h",'

    positional = []
    buff = []
    with open(filename) as fr:
        for line in fr:
            if 'return get_argument_metadata()' in line:
                positional.append(line)
                if not line.strip().endswith(';'):
                    for line in fr:
                        positional.append(line)
                        if line.strip().endswith(';'):
                            break


            if 'symbol' in line:
                pattern = '{[ ]+return[ ]+"(\w+)"[ ]*;[ ]*}'
                m = re.search(pattern, line)
                if m:
                    cmdname = m.groups()[0]

            if line.strip() == starts:
                for line in fr:
                    buff.append(line)
                    if line.strip().endswith(';'):
                        break
                break

    positional = ''.join(positional)
    pattern = '\\.add\\("([^,]+)",[ ]*([-]?\d+)\\)'

    pos_params = []
    for m in re.finditer(pattern, positional):
        params = m.groups()[0]
        pos_params.append(params)


    buff = ''.join(buff)
    pattern = '\s+\\(\n([\s\S]+?)\s+\\)[;\n]{1}'

    param_details = []
    for m in re.finditer(pattern, buff):
        #print buff[m.start(): m.end()]
        params = m.groups()[0].split(',\n')
        assert len(params) == 3
        param_name = get_paraname(params[0])
        param_def  = get_paradef(params[1])
        param_desc = params[2].strip().replace('\\\n', '')

        param_desc = macro_expand(param_desc)
        param_details.append( (param_name, param_def, param_desc) )

    # generate SDK
    #generate_py_SDK(cmdname, pos_params, param_details)
    generate_go_SDK(cmdname, pos_params, param_details)
    #generate_DotNet_SDK(cmdname, pos_params, param_details)

def generate_py_SDK(cmdname, pos_params, params):
    global special_cmds
    template = '''
@mvs_api_v2
def %(cmdname)s(%(argument_defines)s):
    \'\'\'
%(comments)s
    \'\'\'
    positional=[%(positional_arguments)s]
    %(token_arguments)s
    optional={
        %(optional_arguments)s
    }
    return '%(cmdname)s', positional, optional
'''
    def cpp2py_type(cpp_type):
        convert = {
            'uint8_t' : 'int',
            'uint16_t': 'int',
            'uint32_t': 'int',
            'std::uint32_t' : 'int',
            'uint64_t': 'int',

            'int8_t': 'int',
            'int16_t': 'int',
            'int32_t': 'int',
            'std::int32_t': 'int',
            'int64_t': 'int',

            'non_negative_uint64' : 'int',

            'std::string' : 'str',
            'bool': 'bool',
            'explorer::config::transaction' : 'str',
            'std::vector<std::string>' : '[str1, str2, ...]',
            'explorer::config::language' : "string of hexcode",

            'libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>' : "(int_low, int_high)",
            'bc::config::hash256' : "string of hash256",
            'boost::filesystem::path': 'string of file path',
            'bc::wallet::payment_address': 'string of Base58-encoded public key address',
        }
        return convert[cpp_type]

    comments = []
    argument_defines_1 = [] # with no default valie
    argument_defines_2 = [] #  with default value
    optional_arguments = []
    positional_arguments = []
    token_arguments = []
    for param_name, param_def, param_desc in params:
        comments.append("    :param: %s(%s): %s" % (param_name, cpp2py_type(param_def["para_type"]), param_desc))
        if param_def.get("must_give", False) == False:
            if "colon_delimited2_item" in param_def["para_type"]:
                argument_defines_2.append('%s=(0,0)' % param_name)
                special_cmds.append(cmdname)
            else:
                argument_defines_2.append('%s=None' % param_name)
        else:
            argument_defines_1.append('%s' % param_name)

        if param_name in pos_params:
            if param_def["para_type"] == 'std::vector<std::string>':
                positional_arguments.append("' '.join(%s)" % param_name)
                special_cmds.append(cmdname)
            else:
                positional_arguments.append(param_name)
            continue

        if param_def.get("just_token", False) == True:
            assert param_def["para_type"] == "bool"
            token_arguments.append(param_name)
            special_cmds.append(cmdname)
            continue

        if "colon_delimited2_item" in param_def["para_type"]:
            optional_arguments.append(
                '"%s" : "%%s:%%s" %% (%s[0], %s[1]),' % (param_name, param_name, param_name)
            )
        else:
            optional_arguments.append('"%s" : %s,' % (param_name, param_name))

    paras = {
        'comments': '\n'.join(comments),
        'cmdname': cmdname,
        'argument_defines': ', '.join(argument_defines_1 + argument_defines_2),
        'optional_arguments': '\n        '.join(optional_arguments),
        'positional_arguments': ', '.join(positional_arguments),
        'token_arguments': '\n    '.join('if %s == True: positional.append("--%s")' % (i, i) for i in token_arguments),
    }

    print template % paras

def generate_go_SDK(cmdname, pos_params, params):
    template = '''
/*
%(comments)s
*/
func (r *RPCClient) %(func_name)s(%(argument_defines)s) (*JSONRpcResp, error) {
    cmd := "%(cmdname)s"
    positional := []interface{}{%(positional_arguments)s}
    %(positional_defaults)s
    optional := map[string]interface{}{
        %(optional_mustgive)s
    }
    %(token_arguments)s
    %(optional_arguments)s
    args := append(positional, optional)
    return r.doPost(r.Url, cmd, args)
}
'''
    def cpp2go_type(cpp_type):
        convert = {
            'uint8_t' : 'uint8',
            'uint16_t': 'uint16',
            'uint32_t': 'uint32',
            'std::uint32_t' : 'uint32',
            'uint64_t': 'uint64',

            'int8_t': 'int8',
            'int16_t': 'int16',
            'int32_t': 'int32',
            'std::int32_t': 'int32',
            'int64_t': 'int64',

            'non_negative_uint64' : 'uint64',

            'std::string' : 'string',
            'bool': 'bool',
            'explorer::config::transaction' : 'string',
            'std::vector<std::string>' : '[]string',
            'explorer::config::language' : 'string',

            'libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>' : "[2]uint64",
            'bc::config::hash256' : "string",
            'boost::filesystem::path': 'string',
            'bc::wallet::payment_address': 'string',
        }
        return convert[cpp_type]

    def special_type_desc(cpp_type):
        convert = {
            'explorer::config::transaction': "string of hexcode",
            'std::vector<std::string>': 'list of string',
            'libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>' : 'a range expressed by 2 integers',
            'bc::config::hash256' : 'string of hash256',
            'boost::filesystem::path' : 'string of file path',
            'bc::wallet::payment_address' : 'string of Base58-encoded public key address',
        }

        return convert.get(cpp_type, cpp_type)

    def get_zero(go_type):
        if '[2]uint64' == go_type:
            return '[2]uint64{0, 0}'
        if 'int' in go_type:
            return '0'
        if 'string' == go_type:
            return '""'
        if 'bool' == go_type:
            return 'false'
        if '[]string'== go_type:
            return 'nil'
        print go_type
        assert (False)
    comments = []
    argument_defines = []
    token_arguments = []
    optional_arguments = []
    optional_mustgive = []
    positional_arguments = []
    positional_defaults = []
    for param_name, param_def, param_desc in params:
        #print cmdname, param_name
        comments.append("    :param: %s(%s): %s" % (param_name, special_type_desc( param_def["para_type"] ), param_desc) )
        argument_defines.append( '%s %s' % (param_name, cpp2go_type( param_def["para_type"] ) ) )

        if param_name in pos_params:
            if param_def.get("must_give", False) == False:
                zero = get_zero(cpp2go_type(param_def["para_type"]))
                if param_def.has_key("default_value"):
                    if param_def["default_value"] == zero:
                        positional_defaults.append('if %s != %s {\n        positional = append(positional, %s)\n    }' % (
                        param_name, zero, param_name))
                        continue

            if param_def["para_type"] == 'std::vector<std::string>':
                positional_arguments.append('strings.Join(%s, " ")' % param_name)
            else:
                positional_arguments.append(param_name)
            continue
        if param_def.get("just_token", False) == True:
            assert param_def["para_type"] == "bool"
            token_arguments.append(param_name)
            continue

        if param_def.get("must_give", False) == False:
            if "colon_delimited2_item" in param_def["para_type"]:
                optional_arguments.append( (param_name, get_zero(cpp2go_type(param_def["para_type"])), 'strings.Join([]string{strconv.FormatUint(uint64(%s[0]), 10), strconv.FormatUint(uint64(%s[1]), 10)}, ":")' % ( param_name, param_name)))
            else:
                optional_arguments.append( (param_name, get_zero(cpp2go_type(param_def["para_type"])), param_name) )
        else:
            optional_mustgive.append('"%s":%s,' % (param_name, param_name))
    paras = {
        'comments' : '\n'.join(comments),
        'func_name' : cmdname.title(),
        'cmdname' : cmdname,
        'argument_defines' : ', '.join(argument_defines),
        'token_arguments' : '\n    '.join('if %s == true{\n        positional=append(positional, "--%s")\n    }' % (i, i) for i in token_arguments),
        'optional_arguments' : '\n    '.join(['if %s != %s {\n        optional["%s"] = %s\n    }' % (key, zero, key, value) for key, zero, value in optional_arguments]),
        'optional_mustgive' : '\n        '.join(optional_mustgive),
        'positional_arguments' : ', '.join(positional_arguments),
        'positional_defaults' : '\n    '.join(positional_defaults),
    }

    print template % paras


def generate_DotNet_SDK(cmdname, pos_params, params):
    template = '''
/*
%(comments)s
*/
public %(rettype)s %(func_name)s(%(argument_defines)s)
{
    List<String> parameters = new List<String>() { %(positional_arguments)s };
    %(positional_arguments_with_default_value)s
    %(token_arguments)s
    %(add_optional_parameters)s
    return getResult<%(rettype)s>("%(func_name)s", parameters);
}
'''
    def cpp2dotnet_type(cpp_type):
        convert = {
            'uint8_t' : 'UInt8?',
            'uint16_t': 'UInt16?',
            'uint32_t': 'UInt32?',
            'std::uint32_t' : 'UInt32?',
            'uint64_t': 'UInt64?',

            'int8_t': 'Int8?',
            'int16_t': 'Int16?',
            'int32_t': 'Int32?',
            'std::int32_t': 'Int32?',
            'int64_t': 'Int64?',

            'non_negative_uint64' : 'UInt64?',

            'std::string' : 'String',
            'bool': 'Boolean?',
            'explorer::config::transaction' : 'String',
            'std::vector<std::string>' : 'List<String>',
            'explorer::config::language' : 'String',

            'libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>' : "Tuple<UInt64,UInt64>",
            'bc::config::hash256' : "String",
            'boost::filesystem::path': 'String',
            'bc::wallet::payment_address': 'String',
        }
        return convert[cpp_type]

    def special_type_desc(cpp_type):
        convert = {
            'explorer::config::transaction': "string of hexcode",
            'std::vector<std::string>': 'list of string',
            'libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>' : 'a range expressed by 2 integers',
            'bc::config::hash256' : 'string of hash256',
            'boost::filesystem::path' : 'string of file path',
            'bc::wallet::payment_address' : 'string of Base58-encoded public key address',
        }

        return convert.get(cpp_type, cpp_type)
    comments = []
    argument_defines_1 = [] # with no default valie
    argument_defines_2 = [] #  with default value
    optional_arguments = []
    positional_arguments = []
    positional_arguments_with_default_value = []
    token_arguments = []
    for param_name, param_def, param_desc in params:
        comments.append("    :param: %s(%s): %s" % (param_name, special_type_desc(param_def["para_type"]), param_desc))
        #argument_defines.append('%s %s' % (cpp2dotnet_type(param_def["para_type"]), param_name))
        if param_def.get("must_give", False) == False:
            argument_defines_2.append('%s %s=null' % (cpp2dotnet_type(param_def["para_type"]), param_name))
        else:
            argument_defines_1.append('%s %s' %  (cpp2dotnet_type(param_def["para_type"]).replace('?', ''), param_name) )

        if param_name in pos_params:
            if param_def.get("must_give", False) == False:
                target = positional_arguments_with_default_value
            else:
                target = positional_arguments

            if cpp2dotnet_type(param_def["para_type"]) == 'String':
                target.append((param_name, param_name))
            elif cpp2dotnet_type(param_def["para_type"]) == 'List<String>':
                target.append((param_name, 'String.Join(" ", %s.ToArray())' % param_name))
            else:
                target.append((param_name, '%s.ToString()' % param_name))
            continue

        if param_def.get("just_token", False) == True:
            assert param_def["para_type"] == "bool"
            token_arguments.append(param_name)
            continue

        if "vector<" in param_def["para_type"]:
            optional_arguments.append('foreach (var i in %s) {' % param_name)
            optional_arguments.append('    parameters.AddRange(new List<String>{"--%s", i.ToString()});' % param_name)
            optional_arguments.append('}')
        else:
            prefix = ""
            if param_def.get("must_give", False) == False:
                prefix = "if (%s != null) " % param_name

            if "colon_delimited2_item" in param_def["para_type"]:
                value = 'parameters.AddRange(new List<String>{"--%s", String.Format("{0}:{1}", %s.Item1, %s.Item2)});' % (param_name, param_name, param_name)
            else:
                value = 'parameters.AddRange(new List<String>{"--%s", %s.ToString()});' % (param_name, param_name)
            optional_arguments.append(prefix + value)

    paras = {
        'comments': '\n'.join(comments),
        'func_name': cmdname,
        'argument_defines': ', '.join(argument_defines_1 + argument_defines_2),
        'add_optional_parameters': '\n    '.join(optional_arguments),
        'positional_arguments': ', '.join([i[1] for i in positional_arguments]),
        'positional_arguments_with_default_value' : '\n    '.join([ 'if (%s != null) parameters.Add(%s);' % (i[0], i[1]) for i in positional_arguments_with_default_value ] ),
        'rettype' : func2type[cmdname],
        'token_arguments': '\n    '.join([ 'if (%s == true) parameters.Add("--%s");' % (i, i) for i in token_arguments ])
    }

    print template % paras

if __name__ == "__main__":
    for root, dirs, files in os.walk(cmd_dir):
        for file in files:
            parse_cmd(root + '/' + file)
    print >> sys.stderr, set(special_cmds)