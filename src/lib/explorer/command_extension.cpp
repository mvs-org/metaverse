/**
 * Copyright (c) 2016 mvs developers 
 *
 * This file is part of libbitcoin-explorer.
 *
 * libbitcoin-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>
#include <bitcoin/explorer/define.hpp>
#include <bitcoin/explorer/callback_state.hpp>
#include <bitcoin/explorer/display.hpp>
#include <bitcoin/explorer/prop_tree.hpp>
#include <bitcoin/explorer/dispatch.hpp>
#include <bitcoin/explorer/command_extension.hpp>
#include <bitcoin/explorer/command_extension_func.hpp>
#include <bitcoin/explorer/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ stopall *************************/
console_result stopall::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    if(!blockchain.is_admin_account(auth_.name))
        throw std::logic_error{"not admin account!"};
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    output << "mvs server stoped.";
    killpg(getpid(),SIGTERM);
    return console_result::okay;
}

/************************ stop *************************/

console_result stop::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    auto ret = miner.stop();
    if (ret) {
        output<<"mining stoped.";
        return console_result::okay;
    } else {
        output<<"mining stoped got error.";
        return console_result::failure;
    }
}

/************************ setminingaccount *************************/

console_result setminingaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr) 
        throw std::logic_error{"nullptr for address list"};

#if 0 // no random address required for miner
    auto pubkey = pvaddr->begin()->get_pub_key();
#else
    auto is_found = blockchain.get_account_address(auth_.name, argument_.payment_address.encoded());
    if (!is_found)
        throw std::logic_error{"address does not match account."};
#endif

    auto ret = miner.set_miner_payment_address(argument_.payment_address);
    if (ret) {
        output<<"setting address ["<<argument_.payment_address.encoded()<<"] successfully.";
        return console_result::okay;
    } else {
        output<<"got error.";
        return console_result::failure;
    }
}


/************************ getwork *************************/

console_result getwork::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    std::string seed_hash;
    std::string header_hash;
    std::string boundary;
    auto ret = miner.get_work(seed_hash, header_hash, boundary);

    if (ret) {
        pt::ptree aroot;

        aroot.put("id", 1);
        aroot.put("jsonrpc", "1.0");
        
        pt::ptree result;
        result.push_back(std::make_pair("", pt::ptree().put("", header_hash)));
        result.push_back(std::make_pair("", pt::ptree().put("", seed_hash)));
        result.push_back(std::make_pair("", pt::ptree().put("", boundary)));

        aroot.add_child("result", result);
        pt::write_json(output, aroot);

        return console_result::okay;
    } else {
        output<<"no address setting.";
        return console_result::failure;
    }
}

/************************ submitwork *************************/

console_result submitwork::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    auto ret = miner.put_result(argument_.nounce, argument_.mix_hash, argument_.header_hash);
    pt::ptree root;

    root.put("id", 1);
    root.put("jsonrpc", "1.0");

    if (ret) {
        root.put("result", true);
        pt::write_json(output, root);
        return console_result::okay;
    } else {
        root.put("result", false);
        pt::write_json(output, root);
        return console_result::failure;
    }
}

/************************ submithashrate *************************/

console_result submithashrate::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    output << IN_DEVELOPING;
    return console_result::failure;
}


/************************ start *************************/

console_result start::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    std::istringstream sin;
    std::ostringstream sout;
    
    // get new address 
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};
    sin.str("");
    sout.str("");
    dispatch_command(3, cmds2 , sin, sout, sout, blockchain);
    auto&& str_addr = sout.str();
    bc::wallet::payment_address addr(str_addr);

    // start
    if (miner.start(addr)){
        output<<"solo mining started at "<<str_addr<<".";
        return console_result::okay;
    } else {
        output<<"solo mining startup got error.";
        return console_result::failure;
    }
}


/************************ getinfo *************************/

console_result getinfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ getpeerinfo *************************/

console_result getpeerinfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ ping *************************/

console_result ping::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ addnode *************************/

console_result addnode::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ getmininginfo *************************/

console_result getmininginfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    pt::ptree aroot;
    pt::ptree info;
    
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    uint64_t height, rate;
    std::string difficulty;
    bool is_mining;
    
    miner.get_state(height, rate, difficulty, is_mining);
    info.put("status", is_mining);
    info.put("height", height);
    info.put("rate", rate);
    info.put("difficulty", difficulty);
    aroot.push_back(std::make_pair("mining-info", info));
    pt::write_json(output, aroot);

    return console_result::okay;
}
/************************ fetchheaderext *************************/

console_result fetchheaderext::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    using namespace libbitcoin::config; // for hash256
    
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(argument_.number.empty())
        throw std::logic_error{"Block number or earliest, latest, pending is needed"};

    chain::header block_header;

    auto ret = miner.get_block_header(block_header, argument_.number);

    pt::ptree aroot;
    pt::ptree tree;
    
    if(ret) {
        tree.put("bits", block_header.bits);
        tree.put("hash", hash256(block_header.hash()));
        tree.put("merkle_tree_hash", hash256(block_header.merkle));
        tree.put("nonce", block_header.nonce);
        tree.put("previous_block_hash", hash256(block_header.previous_block_hash));
        tree.put("time_stamp", block_header.timestamp);
        tree.put("version", block_header.version);
        // wdy added
        tree.put("mixhash", block_header.mixhash);
        tree.put("number", block_header.number);
        tree.put("transaction_count", block_header.transaction_count);
    }
    aroot.push_back(std::make_pair("result", tree));
    pt::write_json(output, aroot);

    return console_result::okay;
}

/************************ backupwallet *************************/

console_result backupwallet::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ importwallet *************************/

console_result importwallet::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ lockwallet *************************/

console_result lockwallet::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ backupaccount *************************/

console_result backupaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ importaccount *************************/

console_result importaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    std::istringstream sin("");
    std::ostringstream lang("");
    std::ostringstream sout("");
    
    // parameter account name check
    if (blockchain.is_account_exist(auth_.name))
        throw std::logic_error{"account already exist"};

    if (argument_.words.size() > 24)
        throw std::logic_error{"word count must be less than or equal 24"};
    
    for(auto& i : argument_.words){
        sout<<i<<" ";
    }
    
    auto mnemonic = sout.str().substr(0, sout.str().size()-1); // remove last space
    
    // 1. parameter mnemonic check
    lang<<option_.language;
    sin.str("");
    sout.str("");
    //const char* cmds[256]{"mnemonic-to-seed", "-l", lang.str().c_str()};
    const char* cmds[64]{0x00};
    int i = 0;
    cmds[i++] = "mnemonic-to-seed";
    cmds[i++] = "-l";
    cmds[i++] = lang.str().c_str();
    // 
    for(auto& word : argument_.words){
        cmds[i++] = word.c_str();
    }

    if( console_result::okay != dispatch_command(i, cmds , sin, sout, sout)) {
        output<<sout.str();
        return console_result::failure;
    }
    
    // 2. check mnemonic exist in account database
    auto is_mnemonic_exist = false;
    auto sh_vec = blockchain.get_accounts();
    for(auto& each : *sh_vec) {
        if(mnemonic == each.get_mnemonic()){
            is_mnemonic_exist = true;
            break;
        }
    }
    if(is_mnemonic_exist)
        throw std::logic_error{"mnemonic already exist!"};

    // create account
    auto acc = std::make_shared<bc::chain::account>();
    acc->set_name(auth_.name);
    acc->set_passwd(option_.passwd);
    acc->set_mnemonic(mnemonic);
    //acc->set_hd_index(option_.hd_index); // hd_index updated in getnewaddress
    // flush to db
    blockchain.store_account(acc);

    // generate all account address
    pt::ptree root;
    root.put("name", auth_.name);
    root.put("mnemonic", mnemonic);
    root.put("hd_index", option_.hd_index);
    
    uint32_t idx = 0;
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), option_.passwd.c_str()};
    pt::ptree addresses;
    
    for( idx = 0; idx < option_.hd_index; idx++ ) {
        pt::ptree addr;
        sin.str("");
        sout.str("");
        dispatch_command(3, cmds2 , sin, sout, sout, blockchain);
        addr.put("", sout.str());
        addresses.push_back(std::make_pair("", addr));
    }

    root.add_child("addresses", addresses);
    
    pt::write_json(output, root);
    
    return console_result::okay;
}

/************************ listaccounts *************************/

console_result listaccounts::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_admin_account(auth_.name))
        throw std::logic_error{"you are not admin account!"};
    
    auto sh_vec = blockchain.get_accounts();
    
    pt::ptree root;
    pt::ptree accounts;
    const auto action = [&](account& elem)
    {
        pt::ptree acc;
        acc.put("name", elem.get_name());
        acc.put("mnemonic-key", elem.get_mnemonic());
        acc.put("address-count", elem.get_hd_index());
        acc.put("user-status", elem.get_user_status());
        //root.add_child(elem.get_name(), acc);
        accounts.push_back(std::make_pair("", acc));
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
    root.add_child("accounts", accounts);
    pt::write_json(output, root);
    
    return console_result::okay;
}


/************************ getnewaccount *************************/

console_result getnewaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    if (blockchain.is_account_exist(auth_.name)){
        throw std::logic_error{"account already exist"};
    }

    const char* cmds[]{"seed"};
    //, "mnemonic-to-seed", "hd-new", 
    //    "hd-to-ec", "ec-to-public", "ec-to-address"};
    std::ostringstream sout("");
    std::istringstream sin;
    pt::ptree root;

    auto acc = std::make_shared<bc::chain::account>();

#ifdef NDEBUG
    if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
        auth_.auth.length() > 128 || auth_.auth.length() < 6)
        throw std::logic_error{"name length in [3, 128], password length in [6, 128]"};
#endif

    acc->set_name(auth_.name);
    acc->set_passwd(auth_.auth);

    auto exec_with = [&](int i){
        sin.str(sout.str());
        sout.str("");
        dispatch_command(1, cmds + i, sin, sout, sout);
    };

    exec_with(0);
    const char* cmds3[3]{"mnemonic-new", "-l" , option_.language.c_str()};
    sin.str(sout.str());
    sout.str("");
    dispatch_command(3, cmds3 , sin, sout, sout);

    root.put("mnemonic", sout.str());
    acc->set_mnemonic(sout.str());
    
    // flush to db
    auto ret = blockchain.store_account(acc);

    // get 1 new sub-address by default
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};
    sin.str("");
    sout.str("");
    dispatch_command(3, cmds2 , sin, sout, sout, blockchain);
    root.put("default-address", sout.str());
    
    pt::write_json(output, root);
    return console_result::okay;
}


/************************ getaccount *************************/

console_result getaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    pt::ptree root;
    root.put("name", acc->get_name());
    root.put("mnemonic-key", acc->get_mnemonic());
    root.put("address-count", acc->get_hd_index());
    root.put("user-status", acc->get_user_status());
    pt::write_json(output, root);

    return console_result::okay;
}


/************************ lockaccount *************************/

console_result lockaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ setaccountinfo *************************/

console_result setaccountinfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ getaddress *************************/

console_result getaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ getnewaddress *************************/

console_result getnewaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (acc->get_mnemonic().empty()) { throw std::logic_error("mnemonic empty"); }

    const char* cmds[]{"mnemonic-to-seed", "hd-new", "hd-to-ec", "ec-to-public", "ec-to-address"};
    std::ostringstream sout("");
    std::istringstream sin(acc->get_mnemonic());

    auto exec_with = [&](int i){
        sin.str(sout.str());
        sout.str("");
        return dispatch_command(1, cmds + i, sin, sout, sout);
    };

    auto addr = std::make_shared<bc::chain::account_address>();
    addr->set_name(auth_.name);

    dispatch_command(1, cmds + 0, sin, sout, sout);
    exec_with(1);

    auto&& argv_index = std::to_string(acc->get_hd_index());
    const char* hd_private_gen[3] = {"hd-private", "-i", argv_index.c_str()};
    sin.str(sout.str());
    sout.str("");
    dispatch_command(3, hd_private_gen, sin, sout, sout);

    exec_with(2);
    addr->set_prv_key(sout.str());
    exec_with(3);
    addr->set_pub_key(sout.str());

    // testnet
    if (blockchain.chain_settings().use_testnet_rules){
        const char* cmds_tn[]{"ec-to-address", "-v", "127"};
        sin.str(sout.str());
        sout.str("");
        dispatch_command(3, cmds_tn, sin, sout, sout);
    // mainnet
    } else {
        exec_with(4);
    }

    addr->set_address(sout.str());
    addr->set_status(1); // 1 -- enable address
    output<<sout.str();

    acc->increase_hd_index();
    addr->set_hd_index(acc->get_hd_index());
    blockchain.store_account(acc);
    blockchain.store_account_address(addr);

    return console_result::okay;
}


/************************ listaddresses *************************/

console_result listaddresses::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    pt::ptree aroot;
    pt::ptree addresses;

    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw std::logic_error{"nullptr for address list"};

    for (auto& i: *vaddr){
        pt::ptree address;
        address.put("", i.get_address());
        addresses.push_back(std::make_pair("", address));
    }

    aroot.add_child("addresses", addresses);

    pt::write_json(output, aroot);
    return console_result::okay;
}


/************************ getblock *************************/

console_result getblock::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ signmessage *************************/

console_result signmessage::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ verifymessage *************************/

console_result verifymessage::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ createmultisig *************************/

console_result createmultisig::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ addmultisigaddress *************************/

console_result addmultisigaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ validateaddress *************************/

console_result validateaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    if (!blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address!"};
    output<<"valid address "<<argument_.address;
    return console_result::okay;
}


/************************ listbalances *************************/

console_result listbalances::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    pt::ptree aroot;
    pt::ptree balances;

    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw std::logic_error{"nullptr for address list"};

    const char* cmds[2]{"xfetchbalance", nullptr};
    std::stringstream sout;
    std::istringstream sin; 

    for (auto& i: *vaddr){
        sout.str("");
        cmds[1] = i.get_address().c_str();
        dispatch_command(2, cmds + 0, sin, sout, sout, blockchain);

        pt::ptree address_balance;
        pt::read_json(sout, address_balance);
        balances.push_back(std::make_pair("", address_balance));
    }
    
    aroot.add_child("balances", balances);
    pt::write_json(output, aroot);
    return console_result::okay;
}


/************************ getbalance *************************/

console_result getbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    pt::ptree aroot;

    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw std::logic_error{"nullptr for address list"};

    const char* cmds[2]{"xfetchbalance", nullptr};
    std::ostringstream sout;
    std::istringstream sin; 
    uint64_t total_confirmed = 0;
    uint64_t total_received = 0;
    uint64_t total_unspent = 0;
	uint64_t total_frozen = 0;

    for (auto& i: *vaddr){
        sout.str("");
        cmds[1] = i.get_address().c_str();
        dispatch_command(2, cmds + 0, sin, sout, sout, blockchain);

        pt::ptree utxo;
        sin.str(sout.str());
        pt::read_json(sin, utxo);
        total_confirmed += utxo.get<uint64_t>("balance.confirmed");
        total_received += utxo.get<uint64_t>("balance.received");
        total_unspent += utxo.get<uint64_t>("balance.unspent");
		total_frozen += utxo.get<uint64_t>("balance.frozen");
    }
    
    aroot.put("total-confirmed", total_confirmed);
    aroot.put("total-received", total_received);
    aroot.put("total-unspent", total_unspent);
	aroot.put("total-frozen", total_frozen);
    pt::write_json(output, aroot);

    return console_result::okay;
}


/************************ getbestblockhash *************************/

console_result getbestblockhash::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{

    uint64_t height = 0;
    if(!blockchain.get_last_height(height))
        throw std::logic_error{"query last height failure."};

    auto&& height_str = std::to_string(height);
    const char* cmds[]{"fetch-header", "-t", height_str.c_str()};

    std::ostringstream sout("");
    std::istringstream sin("");

    if (dispatch_command(3, cmds, sin, sout, sout))
        throw std::logic_error(sout.str());

    pt::ptree header;
    sin.str(sout.str());
    pt::read_json(sin, header);

    auto&& blockhash = header.get<std::string>("result.hash");
    output<<blockhash;

    return console_result::okay;
}


/************************ getbestblockheader *************************/

console_result getbestblockheader::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    uint64_t height = 0;
    if(!blockchain.get_last_height(height))
        throw std::logic_error{"query last height failure."};

    auto&& height_str = std::to_string(height);
    const char* cmds[]{"fetch-header", "-t", height_str.c_str()};

    std::ostringstream sout("");
    std::istringstream sin("");

    if (dispatch_command(3, cmds, sin, sout, sout))
        throw std::logic_error(sout.str());

    output<<sout.str();

    return console_result::okay;
}


/************************ listtxs *************************/

console_result listtxs::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    using namespace libbitcoin::config; // for hash256
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // address is required in this command
    if (!argument_.address.empty() && !blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address parameter!"};
    
    pt::ptree aroot;
    pt::ptree balances;
    auto sh_tx_hash = std::make_shared<std::set<std::string>>();
    auto sh_txs = std::make_shared<std::vector<tx_block_info>>();
    // 1. no address -- list all account tx
    if(argument_.address.empty()) { 
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr) 
            throw std::logic_error{"nullptr for address list"};
        
        for (auto& elem: *pvaddr) {
            auto sh_vec = blockchain.get_address_business_record(elem.get_address());
            // scan all kinds of business
            for (auto each : *sh_vec){
                auto ret = sh_tx_hash->insert(hash256(each.point.hash).to_string());
                if(ret.second) { // new item
                    tx_block_info tx;
                    tx.height = each.height;
                    tx.timestamp = each.data.get_timestamp();
                    tx.hash = hash256(each.point.hash).to_string();
                    sh_txs->push_back(tx);
                } 
            }
        }
    } else { // address exist in command
        // timestamp parameter check
        auto has_colon = false;
        for (auto& i : argument_.address){
            if (i==':') {
                has_colon = true;
                break;
            }
        }
        // 2. has timestamp, list all account tx between star:end
        if (has_colon) {
            const auto tokens = split(argument_.address, BX_TX_POINT_DELIMITER);
            if (tokens.size() != 2)
            {
                throw std::logic_error{"timestamp is invalid format(eg : 123:456)!"};
            }
            uint32_t start, end;
            deserialize(start, tokens[0], true);
            deserialize(end, tokens[1], true);

            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if(!pvaddr) 
                throw std::logic_error{"nullptr for address list"};
            
            for (auto& elem: *pvaddr) {
                auto sh_vec = blockchain.get_address_business_record(elem.get_address());
                // scan all kinds of business
                for (auto each : *sh_vec){
                    if((start <= each.data.get_timestamp()) && (each.data.get_timestamp() < end)) {
                        auto ret = sh_tx_hash->insert(hash256(each.point.hash).to_string());
                        if(ret.second) { // new item
                            tx_block_info tx;
                            tx.height = each.height;
                            tx.timestamp = each.data.get_timestamp();
                            tx.hash = hash256(each.point.hash).to_string();
                            sh_txs->push_back(tx);
                        }
                    }
                }
            }
        // 3. list all tx of the address    
        } else {
            auto sh_vec = blockchain.get_address_business_record(argument_.address);
            // scan all kinds of business
            for (auto each : *sh_vec){
                auto ret = sh_tx_hash->insert(hash256(each.point.hash).to_string());
                if(ret.second) { // new item
                    tx_block_info tx;
                    tx.height = each.height;
                    tx.timestamp = each.data.get_timestamp();
                    tx.hash = hash256(each.point.hash).to_string();
                    sh_txs->push_back(tx);
                } 
            }
        }
    }
    // fetch tx according its hash
    const char* cmds[2]{"fetch-tx", nullptr};
    std::stringstream sout;
    std::istringstream sin; 
    std::vector<std::string> vec_ip_addr; // input addr
    std::vector<std::string> vec_op_addr; // output addr

    for (auto& elem: *sh_tx_hash) {
        sout.str("");
        cmds[1] = elem.c_str();
        
        try {
            if(console_result::okay != dispatch_command(2, cmds + 0, sin, sout, sout))
                continue;
        }catch(std::exception& e){
            log::info("listtxs")<<sout.str();
            log::info("listtxs")<<e.what();
            continue;
        }catch(...){
            log::info("listtxs")<<sout.str();
            continue;
        }
        pt::ptree tx;
        sin.str(sout.str());
        pt::read_json(sin, tx);

        auto tx_hash = tx.get<std::string>("transaction.hash");
        auto inputs = tx.get_child("transaction.inputs");
        auto outputs = tx.get_child("transaction.outputs");
        // not found, try next 
        if ((inputs.size() == 0) || (outputs.size() == 0)) {
            continue;
        }

        // found, then push_back
        pt::ptree tx_item;
        for (auto& each: *sh_txs){
            if( each.hash.compare(tx_hash) != 0 )
                continue;

            tx_item.put("height", each.height);
            tx_item.put("timestamp", each.timestamp);
            tx_item.put("direction", "send");

            // set inputs content
            pt::ptree input_addrs;
            for(auto& input : inputs) {
                pt::ptree input_addr;
                std::string addr="";
                try {
                    addr = input.second.get<std::string>("address");
                } catch(...){
                    log::info("listtxs no input address!");
                }
                input_addr.put("address", addr);
                input_addrs.push_back(std::make_pair("", input_addr));

                // add input address
                if(!addr.empty()) {
                    vec_ip_addr.push_back(addr);
                }
            }
            tx_item.push_back(std::make_pair("inputs", input_addrs));
            
            // set outputs content
            pt::ptree pt_outputs;
            for(auto& op : outputs) {
                pt::ptree pt_output;
                pt_output.put("address", op.second.get<std::string>("address"));
                pt_output.put("etp-value", op.second.get<uint64_t>("value"));
                //pt_output.add_child("attachment", op.second.get<pt::ptree>("attachment"));
                pt_output.add_child("attachment", op.second.get_child("attachment"));
                pt_outputs.push_back(std::make_pair("", pt_output));
                
                // add output address
                vec_op_addr.push_back(op.second.get<std::string>("address"));
            }
            tx_item.push_back(std::make_pair("outputs", pt_outputs));
            
            // set tx direction
            // 1. receive check
            auto pos = std::find_if(vec_ip_addr.begin(), vec_ip_addr.end(), [&](const std::string& i){
                    return blockchain.get_account_address(auth_.name, i) != nullptr;
                    });
            
            if (pos == vec_ip_addr.end()){
                tx_item.put("direction", "receive");
            }
            // 2. transfer check
            #if 0
            auto is_ip_intern = true;
            auto is_op_intern = true;

            if(vec_ip_addr.empty())
                is_ip_intern = false;
            for(auto& each : vec_ip_addr) {
                if(!blockchain.get_account_address(auth_.name, each))
                    is_ip_intern = false;
            }
            
            for(auto& each : vec_op_addr) {
                if(!blockchain.get_account_address(auth_.name, each))
                    is_op_intern = false;
            }
            
            if (is_ip_intern && is_ip_intern){
                tx_item.put("direction", "transfer");
            }
            #endif
            // 3. all address clear
            vec_ip_addr.clear();
            vec_op_addr.clear();
        }
        balances.push_back(std::make_pair("", tx_item));
    }
    
    aroot.add_child("transactions", balances);
    pt::write_json(output, aroot);

    return console_result::okay;
}


/************************ gettx *************************/

console_result gettx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ getaddresstx *************************/

console_result getaddresstx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ getaccounttx *************************/

console_result getaccounttx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ deposit *************************/
console_result deposit::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};

    if (argument_.deposit != 7 && argument_.deposit != 30 
		&& argument_.deposit != 90 && argument_.deposit != 182
		&& argument_.deposit != 365)
    {
        throw std::logic_error{"deposit must be one in [7, 30, 90, 182, 365]."};
    }

    std::list<prikey_amount> palist;

    const char* cmds[4]{"xfetchbalance", nullptr, "-t", "etp"};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        cmds[1] = each.get_address().c_str();
        dispatch_command(4, cmds + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            palist.push_back({each.get_prv_key(), balance});
        }
    }

    // sort
    palist.sort([](const prikey_amount& first, const prikey_amount& last){
            return first.second < last.second;
            });

    auto random = bc::pseudo_random();
    auto index = random % pvaddr->size();

    // my change
    std::vector<std::string> receiver{
        {pvaddr->at(index).get_address() + ":" + std::to_string(argument_.amount)},
        {pvaddr->at(index).get_address() + ":" + std::to_string(argument_.fee)}
    };

    utxo_helper utxo(std::move(palist), std::move(receiver));
    utxo.set_testnet_rules(blockchain.chain_settings().use_testnet_rules);
    utxo.set_reward(argument_.deposit);

    // send
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}

/************************ send *************************/

console_result send::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};

    std::list<prikey_amount> palist;

    const char* cmds[4]{"xfetchbalance", nullptr, "-t", "etp"};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        cmds[1] = each.get_address().c_str();
        dispatch_command(4, cmds + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            palist.push_back({each.get_prv_key(), balance});
        }
    }

    auto random = bc::pseudo_random();

    // random sort
    palist.sort([random](const prikey_amount& first, const prikey_amount& last){
            return first.second < last.second;
            });

    auto index = random % pvaddr->size();

    // my change
    std::vector<std::string> receiver{
        {argument_.address + ":" + std::to_string(argument_.amount)},
        {pvaddr->at(index).get_address() + ":" + std::to_string(argument_.fee)}
    };

    utxo_helper utxo(std::move(palist), std::move(receiver));
    utxo.set_testnet_rules(blockchain.chain_settings().use_testnet_rules);

    // send
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}


/************************ sendmore *************************/

console_result sendmore::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};

    std::list<prikey_amount> palist;

    const char* cmds[4]{"xfetchbalance", nullptr, "-t", "etp"};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        cmds[1] = each.get_address().c_str();
        dispatch_command(4, cmds + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            palist.push_back({each.get_prv_key(), balance});
        }
    }

    // sort
    palist.sort([](const prikey_amount& first, const prikey_amount& last){
            return first.second < last.second;
            });

    auto random = bc::pseudo_random();
    auto index = random % pvaddr->size();

    // my change
    std::string&& mychange = pvaddr->at(index).get_address() + ":" + std::to_string(argument_.fee);
    argument_.receivers.push_back(mychange);

    utxo_helper utxo(std::move(palist), std::move(argument_.receivers));
    utxo.set_testnet_rules(blockchain.chain_settings().use_testnet_rules);
    // send
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}


/************************ sendfrom *************************/



console_result sendfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw std::logic_error{"invalid from address!"};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw std::logic_error{"invalid to address!"};
    
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};

    std::list<prikey_amount> palist;

    const char* cmds[4]{"xfetchbalance", nullptr, "-t", "etp"};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        if(each.get_address().compare(argument_.from) != 0)
            continue;
        
        sout.str("");
        cmds[1] = each.get_address().c_str();
        dispatch_command(4, cmds + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance && (balance >= argument_.fee)){
            palist.push_back({each.get_prv_key(), balance});
        }
    }
    if(palist.empty())
        throw std::logic_error{"not enough etp in from address or you are't own from address!"};
    
    // my change
    std::vector<std::string> receiver{
        {argument_.to + ":" + std::to_string(argument_.amount)},
        {argument_.from + ":" + std::to_string(argument_.fee)}
    };

    utxo_helper utxo(std::move(palist), std::move(receiver));
    utxo.set_testnet_rules(blockchain.chain_settings().use_testnet_rules);

    // send
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}



/************************ sendwithmsg *************************/

console_result sendwithmsg::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ sendwithmsgfrom *************************/

console_result sendwithmsgfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ listassets *************************/

console_result listassets::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    pt::ptree aroot;
    pt::ptree assets;
    if(auth_.name.empty()) { // no account -- list whole assets in blockchain
        //blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        // std::shared_ptr<std::vector<asset_detail>> 
        auto sh_vec = blockchain.get_issued_assets();
        
        if ( 0 == sh_vec->size()) // no asset found
            throw std::logic_error{"no asset found ?"};

#ifdef MVS_DEBUG
        const auto action = [&](asset_detail& elem)
        {
            log::info("listassets blockchain") << elem.to_string();
        };
        std::for_each(sh_vec->begin(), sh_vec->end(), action);
#endif

        // add blockchain assets
        for (auto& elem: *sh_vec) {
            pt::ptree asset_data;
            asset_data.put("symbol", elem.get_symbol());
            asset_data.put("amount", elem.get_maximum_supply());
            asset_data.put("address", elem.get_address());
            asset_data.put("status", "issued");
            assets.push_back(std::make_pair("", asset_data));
        }
        
    } else { // list asset owned by account
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr) 
            throw std::logic_error{"nullptr for address list"};
        
        // 1. get asset in blockchain
        auto kind = business_kind::asset_transfer;
        std::set<std::string> symbol_set;
        std::vector<asset_detail> asset_vec; // just used asset_detail class to store asset infor

        // make all asset kind vector
        std::vector<business_kind> kind_vec;
        kind_vec.push_back(business_kind::asset_transfer);
        kind_vec.push_back(business_kind::asset_issue);
        
        for (auto kind : kind_vec) {
            // get address unspent asset balance
            for (auto& each : *pvaddr){
                auto sh_vec = blockchain.get_address_business_history(each.get_address(), kind, business_status::unspent);
                const auto sum = [&](const business_history& bh)
                {
                    // get asset info
                    std::string symbol;
                    uint64_t num;
                    if(kind == business_kind::asset_transfer) {
                        auto transfer_info = boost::get<asset_transfer>(bh.data.get_data());
                        symbol = transfer_info.get_address();
                        num = transfer_info.get_quantity();
                    } else { // asset issued
                        auto asset_info = boost::get<asset_detail>(bh.data.get_data());
                        symbol = asset_info.get_symbol();
                        num = asset_info.get_maximum_supply();
                    }
                    
                    // update asset quantity
                    auto r = symbol_set.insert(symbol);
                    if(r.second) { // new symbol
                        asset_vec.push_back(asset_detail(symbol, num, 0, "", each.get_address(), ""));
                    } else { // already exist
                        const auto add_num = [&](asset_detail& elem)
                        {
                            if( 0 == symbol.compare(elem.get_symbol()) )
                                elem.set_maximum_supply(elem.get_maximum_supply()+num);
                        };
                        std::for_each(asset_vec.begin(), asset_vec.end(), add_num);
                    }
                };
                std::for_each(sh_vec->begin(), sh_vec->end(), sum);
            }
        } 
        
        for (auto& elem: asset_vec) {
            pt::ptree asset_data;
            asset_data.put("symbol", elem.get_symbol());
            asset_data.put("amount", elem.get_maximum_supply());
            asset_data.put("address", elem.get_address());
            asset_data.put("status", "unspent");
            assets.push_back(std::make_pair("", asset_data));
        }
        // 2. get asset in local database
        // shoudl filter all issued asset which be stored in local account asset database
        auto sh_vec = blockchain.get_issued_assets();
        for (auto& elem: *sh_vec) {
            symbol_set.insert(elem.get_symbol());
        }
        //std::shared_ptr<std::vector<business_address_asset>>
        auto sh_unissued = blockchain.get_account_unissued_assets(auth_.name);        
        for (auto& elem: *sh_unissued) {
            
            auto symbol = elem.detail.get_symbol();
            auto r = symbol_set.insert(symbol);
            if(!r.second) { // asset already issued in blockchain
                continue; 
            }

            pt::ptree asset_data;
            asset_data.put("symbol", elem.detail.get_symbol());
            asset_data.put("amount", elem.detail.get_maximum_supply());
            asset_data.put("address", "");
            asset_data.put("status", "unissued");
            assets.push_back(std::make_pair("", asset_data));
        }
    }
    
    aroot.add_child("assets", assets);
    pt::write_json(output, aroot);
    return console_result::okay;
}


/************************ getasset *************************/

console_result getasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.size() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol exceed length ?"};

    // 1. first search asset in blockchain
    // std::shared_ptr<std::vector<asset_detail>> 
    auto sh_vec = blockchain.get_issued_assets();
    auto sh_local_vec = blockchain.get_local_assets();
#ifdef MVS_DEBUG
    const auto action = [&](asset_detail& elem)
    {
        log::info("getasset blockchain") << elem.to_string();
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
#endif
    //if(sh_vec.empty() && sh_local_vec.empty()) // not found any asset
        //throw std::logic_error{"no asset found ?"};
#ifdef MVS_DEBUG
    const auto lc_action = [&](asset_detail& elem)
    {
        log::info("getasset local") << elem.to_string();
    };
    std::for_each(sh_local_vec->begin(), sh_local_vec->end(), lc_action);
#endif
    
    pt::ptree aroot;
    pt::ptree assets;
    
    // add blockchain assets
    for (auto& elem: *sh_vec) {
        if( elem.get_symbol().compare(argument_.symbol) != 0 )// not request asset symbol
            continue;
        pt::ptree asset_data;
        asset_data.put("symbol", elem.get_symbol());
        asset_data.put("maximum_supply", elem.get_maximum_supply());
        asset_data.put("asset_type", elem.get_asset_type());
        asset_data.put("issuer", elem.get_issuer());
        asset_data.put("address", elem.get_address());
        asset_data.put("description", elem.get_description());
        asset_data.put("status", "issued");
        assets.push_back(std::make_pair("", asset_data));
        
        aroot.add_child("assets", assets);
        pt::write_json(output, aroot);
        return console_result::okay;
    }
    
    // add local assets
    for (auto& elem: *sh_local_vec) {
        if( elem.get_symbol().compare(argument_.symbol) != 0 )// not request asset symbol
            continue;
        pt::ptree asset_data;
        asset_data.put("symbol", elem.get_symbol());
        asset_data.put("maximum_supply", elem.get_maximum_supply());
        asset_data.put("asset_type", elem.get_asset_type());
        asset_data.put("issuer", elem.get_issuer());
        asset_data.put("address", elem.get_address());
        asset_data.put("description", elem.get_description());
        asset_data.put("status", "unissued");
        assets.push_back(std::make_pair("", asset_data));
        
        aroot.add_child("assets", assets);
        pt::write_json(output, aroot);
        return console_result::okay;
    }
    
    aroot.add_child("assets", assets);
    pt::write_json(output, aroot);
    return console_result::okay;
}


/************************ getaddressasset *************************/

console_result getaddressasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    pt::ptree aroot;
    pt::ptree assets;
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.address)) 
        throw std::logic_error{"invalid address!"};
    
    // 1. get asset in blockchain
    auto kind = business_kind::asset_transfer;
    std::set<std::string> symbol_set;
    std::vector<asset_detail> asset_vec; // just used asset_detail class

    // make all asset kind vector
    std::vector<business_kind> kind_vec;
    kind_vec.push_back(business_kind::asset_transfer);
    kind_vec.push_back(business_kind::asset_issue);
    
    for (auto kind : kind_vec) {
        // get address unspent asset balance
        auto sh_vec = blockchain.get_address_business_history(argument_.address, kind, business_status::unspent);
        const auto sum = [&](const business_history& bh)
        {
            // get asset info
            std::string symbol;
            uint64_t num;
            if(kind == business_kind::asset_transfer) {
                auto transfer_info = boost::get<asset_transfer>(bh.data.get_data());
                symbol = transfer_info.get_address();
                num = transfer_info.get_quantity();
            } else { // asset issued
                auto asset_info = boost::get<asset_detail>(bh.data.get_data());
                symbol = asset_info.get_symbol();
                num = asset_info.get_maximum_supply();
            }
            
            // update asset quantity
            auto r = symbol_set.insert(symbol);
            if(r.second) { // new symbol
                asset_vec.push_back(asset_detail(symbol, num, 0, "", argument_.address, ""));
            } else { // already exist
                const auto add_num = [&](asset_detail& elem)
                {
                    if( 0 == symbol.compare(elem.get_symbol()) )
                        elem.set_maximum_supply(elem.get_maximum_supply()+num);
                };
                std::for_each(asset_vec.begin(), asset_vec.end(), add_num);
            }
        };
        std::for_each(sh_vec->begin(), sh_vec->end(), sum);
    } 
    
    for (auto& elem: asset_vec) {
        pt::ptree asset_data;
        asset_data.put("symbol", elem.get_symbol());
        asset_data.put("maximum_supply", elem.get_maximum_supply());
        //asset_data.put("asset_type", elem.detail.get_asset_type());
        //asset_data.put("issuer", elem.detail.get_issuer());
        //asset_data.put("address", elem.detail.get_address());
        //asset_data.put("description", elem.detail.get_description());
        asset_data.put("address", elem.get_address());
        asset_data.put("status", "unspent");
        assets.push_back(std::make_pair("", asset_data));
    }
    
    aroot.add_child("assets", assets);
    pt::write_json(output, aroot);
    return console_result::okay;
}



/************************ getaccountasset *************************/

console_result getaccountasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    
    // std::shared_ptr<std::vector<business_address_asset>>
    auto sh_local_vec = blockchain.get_account_assets();
    if (0 == sh_local_vec->size()) // no asset found
        throw std::logic_error{"no asset found ?"};

#ifdef MVS_DEBUG
    const auto lc_action = [&](business_address_asset& elem)
    {
        log::info("getasset local db") << elem.to_string();
    };
    std::for_each(sh_local_vec->begin(), sh_local_vec->end(), lc_action);
#endif
    
    pt::ptree aroot;
    pt::ptree assets;
    
    // add local database assets
    for (auto& elem: *sh_local_vec) {
        pt::ptree asset_data;
        asset_data.put("symbol", elem.detail.get_symbol());
        asset_data.put("maximum_supply", elem.detail.get_maximum_supply());
        asset_data.put("asset_type", elem.detail.get_asset_type());
        asset_data.put("issuer", elem.detail.get_issuer());
        asset_data.put("address", elem.detail.get_address());
        asset_data.put("description", elem.detail.get_description());
        asset_data.put("status", "unissued");
        assets.push_back(std::make_pair("", asset_data));
    }
    
    aroot.add_child("assets", assets);
    pt::write_json(output, aroot);
    
    return console_result::okay;
}


/************************ createasset *************************/

console_result createasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);
    
    auto ret = blockchain.is_asset_exist(option_.symbol);
    if(ret) 
        throw std::logic_error{"asset symbol is already exist, please use another one"};
    if (option_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    if (option_.address.length() > ASSET_DETAIL_ADDRESS_FIX_SIZE)
        throw std::logic_error{"asset address length must be less than 64."};
    if (!option_.address.empty() && !blockchain.is_valid_address(option_.address))
        throw std::logic_error{"invalid address parameter!"};
    if (option_.description.length() > ASSET_DETAIL_DESCRIPTION_FIX_SIZE)
        throw std::logic_error{"asset description length must be less than 64."};
    if (auth_.name.length() > 64) // maybe will be remove later
        throw std::logic_error{"asset issue(account name) length must be less than 64."};

    auto acc = std::make_shared<asset_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_maximum_supply(option_.maximum_supply);
    acc->set_asset_type(option_.asset_type); // todo -- type not defined
    acc->set_issuer(auth_.name);
    acc->set_address(option_.address);
    acc->set_description(option_.description);
    
    blockchain.store_account_asset(acc);

    //output<<option_.symbol<<" created at local, you can issue it.";
    
    pt::ptree aroot;
    pt::ptree asset_data;
    asset_data.put("symbol", acc->get_symbol());
    asset_data.put("maximum_supply", acc->get_maximum_supply());
    asset_data.put("asset_type", acc->get_asset_type());
    asset_data.put("issuer", acc->get_issuer());
    asset_data.put("address", acc->get_address());
    asset_data.put("description", acc->get_description());
    //asset_data.put("status", "issued");
    aroot.push_back(std::make_pair("asset", asset_data));
        
    pt::write_json(output, aroot);
    
    return console_result::okay;
}


/************************ issue *************************/

console_result issue::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw std::logic_error{"nullptr for address list"};
    
    std::vector<prikey_amount> pavec;

    const char* fetch_cmds[2]{"xfetchbalance", nullptr};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        fetch_cmds[1] = each.get_address().c_str();
        dispatch_command(2, fetch_cmds + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            pavec.push_back({each.get_address(), balance});
        }
    }
    if(!pavec.size())
        throw std::logic_error{"not enough etp in your account!"};

    // get random address    
    auto index = bc::pseudo_random() % pavec.size();
    auto addr = pavec.at(index).first;
    
    // make issuefrom command 
    // eg : issuefrom m m t9Vns3EmKtreq68GEMiq5njs4egGc623hm CAR -f fee
    const char* cmds[256]{0x00};
    int i = 0;
    cmds[i++] = "issuefrom";
    cmds[i++] = auth_.name.c_str();
    cmds[i++] = auth_.auth.c_str();
    cmds[i++] = addr.c_str();
    cmds[i++] = argument_.symbol.c_str();
    cmds[i++] = "-f";
    cmds[i++] = std::to_string(argument_.fee).c_str();

    // exec command
    sin.str("");
    sout.str("");
    
    if (dispatch_command(i, cmds, sin, sout, sout, blockchain))
        throw std::logic_error(sout.str());
    
    output<<sout.str();
    return console_result::okay;
}

/************************ issuefrom *************************/

console_result issuefrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address parameter!"};
    // fail if asset is already in blockchain
    if(blockchain.is_asset_exist(argument_.symbol, false))
        throw std::logic_error{"asset symbol is already exist in blockchain"};
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};
    
    auto sh_vec = blockchain.get_account_asset(auth_.name, argument_.symbol);
    log::debug("issue") << "asset size = " << sh_vec->size();
    if(!sh_vec->size())
        throw std::logic_error{"no such asset"};

#ifdef MVS_DEBUG
    /* debug code begin */    
    const auto action = [&](business_address_asset& elem)
    {
        log::debug("issuefrom") <<elem.to_string();
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
    /* debug code end */    
#endif

    std::list<prikey_amount> palist;

    const char* cmds[2]{"xfetchbalance", nullptr};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        if( 0 == each.get_address().compare(argument_.address) ) {
            sout.str("");
            cmds[1] = each.get_address().c_str();
            dispatch_command(2, cmds + 0, sin, sout, sout, blockchain);

            pt::ptree pt;
            sin.str(sout.str());
            pt::read_json(sin, pt);
			auto unspent = pt.get<uint64_t>("balance.unspent");
			auto frozen = pt.get<uint64_t>("balance.frozen");
			auto balance = unspent - frozen;
            if (balance){
                palist.push_back({each.get_prv_key(), balance});
            }else{
                throw std::logic_error{"no enough balance"};
            }
            break;
    }
    }
    // address check
    if(palist.empty())
        throw std::logic_error{"no such address"};
    
#ifdef MVS_DEBUG
    /* debug code begin */    
    const auto gaction = [&](prikey_amount& elem)
    {
        log::debug("issuefrom") <<"palist="<<elem.first<< " "<<elem.second;
    };
    std::for_each(palist.begin(), palist.end(), gaction);
    /* debug code end */    
#endif
    
    // send
    std::string type("asset-issue");
    auto testnet_rules = blockchain.chain_settings().use_testnet_rules;

    utxo_attach_issuefrom_helper utxo(std::move(auth_.name), std::move(auth_.auth), std::move(type), std::move(palist), 
        std::move(argument_.address), argument_.fee, std::move(argument_.symbol), sh_vec->begin()->quantity, testnet_rules);

    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}



/************************ issuemore *************************/

console_result issuemore::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ issuemorefrom *************************/

console_result issuemorefrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ sendasset *************************/

console_result sendasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw std::logic_error{"nullptr for address list"};
    
    std::vector<prikey_amount> pavec;

    const char* fetch_cmds[2]{"xfetchbalance", nullptr};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        fetch_cmds[1] = each.get_address().c_str();
        dispatch_command(2, fetch_cmds + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            pavec.push_back({each.get_address(), balance});
        }
    }
    if(!pavec.size())
        throw std::logic_error{"not enough etp in your account!"};

    // get random address    
    auto index = bc::pseudo_random() % pavec.size();
    auto addr = pavec.at(index).first;
    
    // make sendassetfrom command 
    // eg : sendassetfrom m m MNH8xif7cx3wWxMoAw2Fj5XTeJ1isHbHt9 MJZWnDUtBVAp6Njuc6734bdrTpMcY1kovc CAR 10
    const char* cmds[256]{0x00};
    int i = 0;
    cmds[i++] = "sendassetfrom";
    cmds[i++] = auth_.name.c_str();
    cmds[i++] = auth_.auth.c_str();
    cmds[i++] = addr.c_str();
    cmds[i++] = argument_.address.c_str();
    cmds[i++] = argument_.symbol.c_str();
    cmds[i++] = std::to_string(argument_.amount).c_str();
    cmds[i++] = "-f";
    cmds[i++] = std::to_string(argument_.fee).c_str();

    // exec command
    sin.str("");
    sout.str("");
    
    if (dispatch_command(i, cmds, sin, sout, sout, blockchain))
        throw std::logic_error(sout.str());
    
    output<<sout.str();
    return console_result::okay;
}

/************************ sendassetfrom *************************/

console_result sendassetfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)

{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    
    if (!blockchain.is_valid_address(argument_.from))
        throw std::logic_error{"invalid from address parameter!"};
    if (!blockchain.is_valid_address(argument_.to))
        throw std::logic_error{"invalid to address parameter!"};
    if (!argument_.amount)
        throw std::logic_error{"invalid asset amount parameter!"};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};
    
    auto kind = business_kind::asset_issue;
    //std::shared_ptr<std::vector<business_history>> 
    auto sh_vec = blockchain.get_address_business_history(argument_.from, argument_.symbol, kind, business_status::unspent);
    log::debug("sendassetfrom") << "asset issue size = " << sh_vec->size();
    if(!sh_vec->size()) { // not found issue asset then search transfer asset

        kind = business_kind::asset_transfer;
        sh_vec = blockchain.get_address_business_history(argument_.from, argument_.symbol, kind, business_status::unspent);
    }
    if(!sh_vec->size()) 
        throw std::logic_error{"the from address has no unspent asset"};
    
#ifdef MVS_DEBUG
    /* debug code begin */    
    const auto action = [&](business_history& elem)
    {
        log::info("sendassetfrom") <<elem.to_string();
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
    /* debug code end */
#endif
    std::list<prikey_etp_amount> asset_ls;
    
    if(kind == business_kind::asset_transfer) {
        
        // get address unspend asset balance
        for (auto& each : *pvaddr){
            if ( 0 != argument_.from.compare(each.get_address()) )
                continue;
            const auto sum = [&](const business_history& bh)
            {
                auto transfer_info = boost::get<asset_transfer>(bh.data.get_data());
                asset_ls.push_back({each.get_prv_key(), bh.value, transfer_info.get_quantity(), bh.output});
            };
            std::for_each(sh_vec->begin(), sh_vec->end(), sum);
            break;
        }
    } else {  // asset_issue 
    
        // get address unspend asset balance
        for (auto& each : *pvaddr){
            if ( 0 != argument_.from.compare(each.get_address()) )
                continue;
            // do data structure exchange
            const auto sum = [&](const business_history& bh)
            {
                auto asset_info = boost::get<asset_detail>(bh.data.get_data());
                asset_ls.push_back({each.get_prv_key(), bh.value, asset_info.get_maximum_supply(), bh.output});
            };
            std::for_each(sh_vec->begin(), sh_vec->end(), sum);
            break;
        }
    }
    if(!asset_ls.size()) 
        throw std::logic_error{"no asset business for from address!"};
    // todo -- add etp business to asset_ls if asset etp not enough
    // todo -- create new etp list
    std::string type("asset-transfer");
    auto testnet_rules = blockchain.chain_settings().use_testnet_rules;

    utxo_attach_sendfrom_helper utxo(std::move(auth_.name), std::move(auth_.auth), std::move(type), std::move(asset_ls), 
        argument_.fee, std::move(argument_.symbol), argument_.amount, std::move(argument_.to), testnet_rules);
    utxo.group_utxo(); // throw exception if etp number or asset amount is not enough
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}


/************************ getdid *************************/

console_result getdid::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ setdid *************************/

console_result setdid::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ sendwithdid *************************/

console_result sendwithdid::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ settxfee *************************/

console_result settxfee::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}

/************************ encodeattachtx *************************/
using namespace bc::explorer::config;
using namespace bc::wallet;

bool encodeattachtx::push_scripts(std::vector<tx_output_type>& outputs,
    const explorer::config::metaverse_output& output_para, uint8_t script_version)
{
    auto output = const_cast<explorer::config::metaverse_output&>(output_para);
    // explicit script
    if (!output.script().operations.empty())
    {
        outputs.push_back({ output.value(), output.script(), output.attach_data() });
        return true;
    }

    // If it's not explicit the script must be a form of pay to short hash.
    if (output.pay_to_hash() == null_short_hash)
        return false;

    chain::operation::stack payment_ops;
    const auto hash = output.pay_to_hash();
    const auto is_stealth = !output.ephemeral_data().empty();

    // This presumes stealth versions are the same as non-stealth.
    if (output.version() != script_version)
        payment_ops = chain::operation::to_pay_key_hash_pattern(hash);
    else if (output.version() == script_version)
        payment_ops = chain::operation::to_pay_script_hash_pattern(hash);
    else
        return false;

    if (is_stealth)
    {
        // Stealth indexing requires an ordered script tuple.
        // The null data script must be pushed before the pay script.
        static constexpr uint64_t no_amount = 0;
        const auto data = output.ephemeral_data();
        const auto null_data = chain::operation::to_null_data_pattern(data);
        const auto null_data_script = chain::script{ null_data };
        outputs.push_back({ no_amount, null_data_script, attachment() });
    }

    const auto payment_script = chain::script{ payment_ops };
    outputs.push_back({ output.value(), payment_script, output.attach_data() });
    return true;
}

void encodeattachtx::refill_output_attach(std::vector<explorer::config::metaverse_output>& vec_cfg_output,
        bc::blockchain::block_chain_impl& blockchain)
{
    for (auto& output: vec_cfg_output) {
        auto& attach = output.attach_data();
#ifdef MVS_DEBUG
        log::debug("command_extension") << "refill_output_attach old attach=" << attach.to_string();
#endif
        if((ASSET_TYPE == attach.get_type())) {
            auto asset_data = boost::get<asset>(attach.get_attach());
            if(ASSET_DETAIL_TYPE == asset_data.get_status()) { // only detail info not complete in metaverse_out
                auto detail = boost::get<asset_detail>(asset_data.get_data());
#ifdef MVS_DEBUG
                log::debug("command_extension") << "refill_output_attach old detail=" << detail.to_string();
#endif
                //std::shared_ptr<std::vector<business_address_asset>>
                auto sh_asset = blockchain.get_account_asset(auth_.name, detail.get_symbol());
                if(sh_asset) {
                    auto ass_vec = *sh_asset;
#ifdef MVS_DEBUG
                    log::debug("command_extension") << "refill_output_attach new detail=" << ass_vec[0].detail.to_string();
#endif
                    ass_vec[0].detail.set_address(detail.get_address()); // target is setted in metaverse_output.cpp
                    asset_data.set_data(ass_vec[0].detail);
                    attach.set_attach(asset_data);
#ifdef MVS_DEBUG
                    log::debug("command_extension") << "refill_output_attach new attach=" << attach.to_string();
#endif
                }
            }
        }
    }
}

console_result encodeattachtx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    // Bound parameters.
    const auto locktime = option_.lock_time;
    const auto tx_version = option_.version;
    const auto script_version = option_.script_version;

    tx_type tx;
    tx.version = tx_version;
    tx.locktime = locktime;

    for (const tx_input_type& input: option_.inputs)
        tx.inputs.push_back(input);
    // refill attach info in output
    refill_output_attach(option_.outputs, blockchain);
    
#ifdef MVS_DEBUG
    for (auto& output: option_.outputs) // todo -- remove (debug code here)
        log::debug("command_extension") << "invoke new attach=" << output.attach_data().to_string();
#endif
    
    for (const auto& output: option_.outputs)
    {
        if (!push_scripts(tx.outputs, output, script_version))
        {
            cerr << BX_ENCODEATTACHTX_INVALID_OUTPUT << std::flush;
            return console_result::failure;
        }
    }

    if (tx.is_locktime_conflict())
    {
        cerr << BX_ENCODEATTACHTX_LOCKTIME_CONFLICT << std::flush;
        return console_result::failure;
    }

    output << transaction(tx) << std::flush;
    return console_result::okay;
}
/************************ changepasswd *************************/

console_result changepasswd::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (acc) {
        blockchain.set_account_passwd(auth_.name, option_.passwd);
    }else{
        throw std::logic_error{"account not found"};
    }

    return console_result::okay;
}

/************************ changepasswdext *************************/

console_result changepasswdext::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    std::istringstream sin("");
    std::ostringstream lang("");
    std::ostringstream sout("");
    
    // parameter check    
    auto user = std::make_shared<bc::chain::account>();

    for(auto& i : argument_.words){
        sout<<i<<" ";
    }
    
    auto mnemonic = sout.str().substr(0, sout.str().size()-1); // remove last space
    
    if(!auth_.name.empty()) {
        auto acc = blockchain.get_account(auth_.name);
        if(!acc)
            throw std::logic_error{"non exist account name"};
        
        if(mnemonic != acc->get_mnemonic())
            throw std::logic_error{"account is not exist for this mnemonic!"};
        
        user = acc;
    } else { // scan all account to compare mnemonic
        auto sh_vec = blockchain.get_accounts();
        for(auto& each : *sh_vec) {
            if(mnemonic == each.get_mnemonic()){
                *user = each;
                break;
            }
        }
    }

    if(mnemonic != user->get_mnemonic())
        throw std::logic_error{"account is not exist for this mnemonic!"};

    lang<<option_.language;
    sin.str("");
    sout.str("");
    //const char* cmds[256]{"mnemonic-to-seed", "-l", lang.str().c_str()};
    const char* cmds[256]{0x00};
    int i = 0;
    cmds[i++] = "mnemonic-to-seed";
    cmds[i++] = "-l";
    cmds[i++] = lang.str().c_str();
    for(auto& word : argument_.words){
        cmds[i++] = word.c_str();
    }

    if( console_result::okay != dispatch_command(i, cmds , sin, sout, sout)) {
        output<<sout.str();
        return console_result::failure;
    }

    user->set_passwd(option_.passwd);
    
    // flush to db
    blockchain.store_account(user);

    return console_result::okay;
}

/************************ xfetchbalance *************************/
console_result xfetchbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
	using namespace bc::client;

	// Bound parameters.
    if (!blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address parameter!"};
	auto addr_str = argument_.address;
	auto address = payment_address(argument_.address);
	auto type = option_.type;
	const auto connection = get_connection(*this);

	obelisk_client client(connection);

	if (!client.connect(connection))
	{
		display_connection_failure(cerr, connection.server);
		return console_result::failure;
	}

	auto on_done = [&addr_str, &type, &output, &blockchain](const history::list& rows)
	{
		pt::ptree tree;
		pt::ptree balance;
		uint64_t total_received = 0;
		uint64_t confirmed_balance = 0;
		uint64_t unspent_balance = 0;
		uint64_t frozen_balance = 0;
		
		uint64_t height = 0;
		blockchain.get_last_height(height);

		for (auto& row: rows)
		{
			total_received += row.value;
		
			// spend unconfirmed (or no spend attempted)
			if (row.spend.hash == null_hash) {
				// fetch utxo script to check deposit utxo
				const char* cmds[]{"fetch-tx"};
				std::ostringstream sout;
				std::istringstream sin;
	            sout.str("");
	            sin.str(encode_hash(row.output.hash));

	            if (dispatch_command(1, cmds, sin, sout, sout))
	                throw std::logic_error(sout.str());
	            sin.str(sout.str());

	            pt::ptree pt;
	            pt::read_json(sin, pt);

	            auto transaction = pt.get_child("transaction");
	            auto outputs = transaction.get_child("outputs");

	            // fill tx_items outputs
	            auto target_pos = row.output.index;
				std::string attah_tp;
	            uint32_t pos = 0;
	            for (auto& i: outputs){
	                if (target_pos == pos++){
	                    std::string script = i.second.get<std::string>("script");
						attah_tp = i.second.get<std::string>("attachment.type");
						bc::chain::script ss;
			            ss.from_string(script);
			            if ((ss.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
							&& !row.output_height) // utxo in transaction pool
							frozen_balance += row.value;
						if(chain::operation::is_pay_key_hash_with_lock_height_pattern(ss.operations)
							&& row.output_height) { // utxo already in block
							uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(ss.operations);
							if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
								frozen_balance += row.value;
							}
						}
	                    break;
	                }
	            }
				if((type == "all") 
					|| ((type == "etp") && (attah_tp == "etp")))
					unspent_balance += row.value;
			}
		
			if (row.output_height != 0 &&
				(row.spend.hash == null_hash || row.spend_height == 0))
				confirmed_balance += row.value;
		}
		#if 0
		uint64_t height = 0;
		blockchain.get_last_height(height);
		auto sh_vec = blockchain.get_address_business_history(addr_str);
		for (auto& row: *sh_vec)
		{
			// spend unconfirmed (or no spend attempted)
			if (row.spend.hash == null_hash) {
				if(business_kind::etp_award == row.data.get_kind_value()) {
					auto award = boost::get<etp_award>(row.data.get_data());
					if((row.output_height+award.get_height())>height) { // deposit not expire
						frozen_balance += row.value;
					}
				}
			}
		
		}
		#endif
		
		balance.put("address", addr_str);
		balance.put("confirmed", confirmed_balance);
		balance.put("received", total_received);
		balance.put("unspent", unspent_balance);
		balance.put("frozen", frozen_balance);
		
		tree.add_child("balance", balance);
		pt::write_json(output, tree);
	};

	auto on_error = [&output](const code& error)
	{
		if(error) {
			output<<error.message();
		}
	};

	// The v3 client API works with and normalizes either server API.
	//// client.address_fetch_history(on_error, on_done, address);
	client.address_fetch_history2(on_error, on_done, address);
	client.wait();

	return console_result::okay;
}

/************************ xfetchutxo *************************/
console_result xfetchutxo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
	using namespace bc::client;

	// Bound parameters.
	if (!blockchain.is_valid_address(argument_.address))
		throw std::logic_error{"invalid address parameter!"};
	auto addr_str = argument_.address;
	auto amount = argument_.amount;
	auto address = payment_address(argument_.address);
	auto type = option_.type;
	const auto connection = get_connection(*this);

	obelisk_client client(connection);

	if (!client.connect(connection))
	{
		display_connection_failure(cerr, connection.server);
		return console_result::failure;
	}

	auto on_done = [&addr_str, &amount, &type, &output, &blockchain](const history::list& rows)
	{
		uint64_t height = 0;
		blockchain.get_last_height(height);
		chain::output_info::list unspent;
		
		for (auto& row: rows)
		{		
			// spend unconfirmed (or no spend attempted)
			if (row.spend.hash == null_hash) {
				// fetch utxo script to check deposit utxo
				const char* cmds[]{"fetch-tx"};
				std::ostringstream sout;
				std::istringstream sin;
				sout.str("");
				sin.str(encode_hash(row.output.hash));

				if (dispatch_command(1, cmds, sin, sout, sout))
					throw std::logic_error(sout.str());
				sin.str(sout.str());

				pt::ptree pt;
				pt::read_json(sin, pt);

				auto transaction = pt.get_child("transaction");
				auto outputs = transaction.get_child("outputs");

				// fill tx_items outputs
				auto target_pos = row.output.index;
				uint32_t pos = 0;
				bool is_deposit_utxo = false;
				std::string attah_tp;
				
				for (auto& i: outputs){
					if (target_pos == pos++){
						std::string script = i.second.get<std::string>("script");
						attah_tp = i.second.get<std::string>("attachment.type");
						bc::chain::script ss;
						ss.from_string(script);
						if ((ss.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
							&& !row.output_height)  // utxo in transaction pool
							is_deposit_utxo = true;
						if(chain::operation::is_pay_key_hash_with_lock_height_pattern(ss.operations)
							&& row.output_height) { // utxo already in block
							uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(ss.operations);
							if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
								is_deposit_utxo = true;
							}
						}
						break;
					}
				}
				if(is_deposit_utxo)
					continue;
				
				if((type == "all") 
					|| ((type == "etp") && (attah_tp == "etp")))
					unspent.push_back({row.output, row.value});
			}
		
		}
		
		chain::points_info selected_utxos;
		wallet::select_outputs::select(selected_utxos, unspent, amount);
			
		pt::ptree tree = prop_tree(selected_utxos, true); // json format
	    pt::write_json(output, tree);
		
	};

	auto on_error = [&output](const code& error)
	{
		if(error) {
			output<<error.message();
		}
	};

	// The v3 client API works with and normalizes either server API.
	//// client.address_fetch_history(on_error, on_done, address);
	client.address_fetch_history2(on_error, on_done, address);
	client.wait();

	return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

