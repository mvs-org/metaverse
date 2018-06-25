/**
 * Copyright (c) 2016-2018 mvs developers
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

#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/commands/importkeyfile.hpp>
#include <metaverse/explorer/extensions/account_info.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/bitcoin/formats/base_64.hpp>
#include <cryptojs/cryptojs_impl.h>

namespace libbitcoin {
namespace explorer {
namespace commands {
namespace fs = boost::filesystem;
using namespace bc::explorer::config;

/************************ importkeyfile *************************/

console_result importkeyfile::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    if (blockchain.is_account_exist(auth_.name)){
        throw account_existed_exception{auth_.name + " account already exist"};
    }

    auto acc = std::make_shared<bc::chain::account>();

    //account name and password
    acc->set_name(auth_.name);
    acc->set_passwd(auth_.auth);

    std::string file_content;
    if (option_.content != "") {
        file_content = option_.content;
    } else {
        fs::file_status status = fs::status(option_.file);
        if (!fs::exists(status)) // not process filesystem exception here
            throw argument_legality_exception{option_.file.string() + std::string(" not exist.")};
        if (fs::is_directory(status)) // directory not allowed
            throw argument_legality_exception{option_.file.string() + std::string(" is directory not a file.")};
        if (!fs::is_regular_file(status))
            throw argument_legality_exception{option_.file.string() + std::string(" not a regular file.")};

        std::ifstream file_input(option_.file.string(), std::ofstream::in);
        file_content.assign(std::istreambuf_iterator<char>(file_input),
                            std::istreambuf_iterator<char>());
        file_input.close();
    }

    Json::Reader reader;
    Json::Value file_root;
    if (file_content.length() && (file_content[0] == '{') && reader.parse(file_content, file_root)){
        //mnemonic
        {
            const std::string encoded_mnemonic = file_root["mnemonic"].asString();
            data_chunk decoded_mnemonic;
            if (!libbitcoin::decode_base64(decoded_mnemonic, encoded_mnemonic)) {
                throw encode_exception{encoded_mnemonic + std::string(" invalid for decode_base64.")};
            }
            std::string mnemonic;
            if (!cryptojs::decrypt(decoded_mnemonic, auth_.auth, mnemonic)) {
                throw encode_exception{std::string("Failed to decrypt mnemonic with password.")};
            }

            if ((mnemonic[0] != '"') || (mnemonic[mnemonic.size()-1] != '"')) {
                throw encode_exception{std::string("decrypted mnemonic format error.")};
            }

            acc->set_mnemonic(mnemonic.substr(1, mnemonic.size()-2), auth_.auth);
        }

        if (blockchain.store_account(acc) != console_result::okay) {
            throw address_generate_exception{std::string("Failed to store account.")};
        }

        //address
        {
            const std::string address_count = file_root["index"].asString();

            // get n new sub-address
            Json::Value jv_temp;
            const char *cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str(), "--number",
                                address_count.c_str()};

            if (dispatch_command(5, cmds2, jv_temp, node, get_api_version()) != console_result::okay) {
                throw address_generate_exception{std::string("Failed to generate address.")};
            }
        }

        //multisig
        {
            const auto &multisigs = file_root["multisigs"];
            const int multisigs_size = multisigs.size();
            for (int i=0; i < multisigs_size; ++i) {
                std::string d = multisigs[i]["d"].asString();
                std::string m = multisigs[i]["m"].asString();
                std::string n = multisigs[i]["n"].asString();
                std::string s = multisigs[i]["s"].asString();

                std::vector<std::string> vec_k;

                const auto &k = multisigs[i]["k"];
                for (unsigned int j=0; j < k.size(); ++j) {
                    std::string d = k[j].asString();
                    vec_k.push_back(d);
                }

                // get n new sub-address
                Json::Value jv_temp;
                std::vector<const char *> vec_cmds = {"getnewmultisig", auth_.name.c_str(), auth_.auth.c_str(),
                                    "-s", s.c_str(),
                                    "-m", m.c_str(),
                                    "-n", n.c_str(),
                                    "-d", d.c_str()
                };
                for (const auto& s : vec_k) {
                    vec_cmds.push_back("-k");
                    vec_cmds.push_back(s.c_str());
                }

                if (dispatch_command(vec_cmds.size(), vec_cmds.data(), jv_temp, node, get_api_version()) != console_result::okay) {
                    throw address_generate_exception{std::string("Failed to generate address.")};
                }
            }
        }

        return console_result::okay;
    }
    else {
        account_info all_info(blockchain, auth_.auth);
        std::stringstream ss(file_content);
        // decrypt account info file first
        ss >> all_info;

        // name check
        auto acc = all_info.get_account();
        auto name = acc.get_name();
        auto mnemonic = acc.get_mnemonic();
        if (blockchain.is_account_exist(name))
            throw account_existed_exception{name + std::string(" account is already exist")};

        // store account info to db
        all_info.store(name, auth_.auth);

        auto& root = jv_output;
        root["name"] = name;

        if (get_api_version() == 1) {
            root["address-count"] += acc.get_hd_index();
            root["unissued-asset-count"] += all_info.get_account_asset().size();
        }
        else if (get_api_version() <= 2) {
            root["address-count"] = acc.get_hd_index();
            root["unissued-asset-count"] = static_cast<uint64_t>(all_info.get_account_asset().size());
        }
        else {
            root["address_count"] = acc.get_hd_index();
            root["unissued_asset_count"] = static_cast<uint64_t>(all_info.get_account_asset().size());
        }

        return console_result::okay;
    }
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

