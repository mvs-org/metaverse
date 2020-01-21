/**
 * Copyright (c) 2016-2020 mvs developers
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
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


#include <metaverse/explorer/extensions/commands/dumpkeyfile.hpp>
#include <metaverse/explorer/extensions/account_info.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/bitcoin/formats/base_64.hpp>
#include <cryptojs/cryptojs_impl.h>

namespace libbitcoin {
namespace explorer {
namespace commands {
namespace fs = boost::filesystem;
using namespace bc::explorer::config;
/************************ exportaccountasfile *************************/

console_result dumpkeyfile::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::string&& mnemonic = blockchain.is_account_lastwd_valid(*acc, auth_.auth, argument_.last_word);

    std::string keyfile_name = "mvs_keystore_" + auth_.name + ".json";

    // account address info
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) throw address_list_nullptr_exception{"nullptr for address list"};

    Json::Value file_root;
    //light-wallet version no,
    //Shared by full-wallet from now on. 2018-04-01
    file_root["version"] = "0.2.1";
    file_root["algo"] = "aes";
    file_root["index"] = uint32_t(pvaddr->size() - acc->get_multisig_vec().size());
    file_root["mnemonic"] = libbitcoin::encode_base64( cryptojs::encrypt("\"" + mnemonic + "\"", auth_.auth) );
    //file_root["accounts"] =  ss.str();

    Json::Value multisig_lst;
    multisig_lst.resize(0);
    for (auto ms : acc->get_multisig_vec()) {
        Json::Value multisig;
        multisig["m"] = ms.get_m();
        multisig["n"] = ms.get_n();
        multisig["s"] = ms.get_pub_key();
        multisig["d"] = ms.get_description();
        for (const auto &cosigner_pubkey : ms.get_cosigner_pubkeys()) {
            multisig["k"].append( cosigner_pubkey );
        }
        multisig_lst.append(multisig);
    }
    file_root["multisigs"] = multisig_lst;

    Json::Value script_lst;
    script_lst.resize(0);
    for (auto sc : acc->get_script_vec()) {
        Json::Value script;
        script["address"] = sc.get_address();
        script["script"] = encode_base16(sc.get_script());
        script["description"] = sc.get_description();

        script_lst.append(script);
    }
    file_root["scripts"] = script_lst;

    Json::Value result;

    if (!option_.is_data) {
        if (argument_.dst.empty()) {
            // default path. provides download.
            if (!boost::filesystem::exists(webpage_path())) {
                fs::create_directory(webpage_path());
            }

            argument_.dst = webpage_path() / "keys";
            fs::create_directory(argument_.dst);
            argument_.dst /= keyfile_name;

        } else {
            std::string dstpath = argument_.dst.string();
            if (dstpath.length() > 0 && dstpath[0] == '~') {
                char *home_dir = getenv("HOME");
                if (home_dir && strlen(home_dir) != 0) {
                    dstpath.replace(0, 1, home_dir);
                    argument_.dst = fs::path(dstpath);
                }
            }

            fs::file_status status = fs::status(argument_.dst);
            if (fs::is_directory(status)) // not process filesystem exception here
                argument_.dst /= keyfile_name;

            fs::file_status status2 = fs::status(argument_.dst.parent_path());
            if (!fs::exists(status2))
                throw argument_legality_exception{
                        argument_.dst.parent_path().string() + std::string(" directory does not exist.")};
        }

        // store encrypted data to file
        bc::ofstream file_output(argument_.dst.string(), std::ofstream::out);
        file_output << file_root.toStyledString() << std::flush;
        file_output.close();

        result = argument_.dst.string();

    } else {
        result = file_root;
    }

    auto& root = jv_output;
    if(get_api_version() == 1)
        root["result"] = result;
    else
        root = result;

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

