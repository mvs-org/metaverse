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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/commands/registermit.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

void registermit::check_symbol_content(const std::string& symbol, const std::string& content)
{
    // check symbol
    if (symbol.size() == 0) {
        throw asset_symbol_length_exception{"Symbol can not be empty."};
    }

    // reserve 4 bytes
    if (symbol.size() > (ASSET_MIT_SYMBOL_FIX_SIZE - 4)) {
        throw asset_symbol_length_exception{"Symbol length must be less than "
            + std::to_string(ASSET_MIT_SYMBOL_FIX_SIZE - 4) + ". " + symbol};
    }

    // check symbol
    check_mit_symbol(symbol, true);

    // check content
    if (content.size() > ASSET_MIT_CONTENT_FIX_SIZE) {
        throw argument_size_invalid_exception(
            "Content length must be less than "
            + std::to_string(ASSET_MIT_CONTENT_FIX_SIZE) + ". " + content);
    }
}

console_result registermit::invoke (Json::Value& jv_output,
        libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::map<std::string, std::string> mit_map;

    bool use_unified_content = false;
    // check single symbol and content
    if (argument_.symbol.size() > 0) {
        check_symbol_content(argument_.symbol, option_.content);

        // check symbol not registered
        if (blockchain.get_registered_mit(argument_.symbol)) {
            throw asset_symbol_existed_exception{"MIT already exists in blockchain. " + argument_.symbol};
        }

        mit_map[argument_.symbol] = option_.content;
    }
    else {
        if (option_.content.size() > 0) {
            // check content
            if (option_.content.size() > ASSET_MIT_CONTENT_FIX_SIZE) {
                throw argument_size_invalid_exception(
                    "Content length must be less than "
                    + std::to_string(ASSET_MIT_CONTENT_FIX_SIZE) + ". " + option_.content);
            }

            use_unified_content = true;
        }
    }

    // check multi symbol and content
    for (const auto& mit : option_.multimits) {
        std::string symbol, content;
        auto pos = mit.find_first_of(":");
        if (pos == std::string::npos) {
            symbol = mit;

            if (use_unified_content) {
                content = option_.content;
            }
            else {
                content = "";
            }
        }
        else {
            symbol = mit.substr(0, pos);
            content = mit.substr(pos + 1);
        }

        check_symbol_content(symbol, content);

        if (mit_map.find(symbol) != mit_map.end()) {
            throw asset_symbol_existed_exception{"Duplicate symbol: " + symbol};
        }

        // check symbol not registered
        if (blockchain.get_registered_mit(symbol)) {
            throw asset_symbol_existed_exception{"MIT already exists in blockchain. " + symbol};
        }

        mit_map[symbol] = content;
    }

    if (mit_map.empty()) {
        throw argument_legality_exception{"No symbol provided."};
    }

    std::string category;
    bool is_create_category = false;
    for (auto& pair : mit_map) {
        auto& symbol = pair.first;
        std::string current_category;
        auto pos = symbol.find(".");
        if (pos != std:: string::npos) {
            current_category = symbol.substr(0, pos);
        }
        else {
            current_category = symbol;
            is_create_category = true;
        }

        if (category.empty()) {
            category = current_category;
        }
        else {
            if (category != current_category) {
                throw argument_legality_exception{"Categories are not the same."};
            }
        }
    }

    if (!is_create_category) {
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr)
            throw address_list_nullptr_exception{"nullptr for address list"};

        // check category MIT registered
        auto info = blockchain.get_registered_mit(category);
        if (!info) {
            throw asset_symbol_notfound_exception{"Category MIT does not exist in blockchain. " + category};
        }

        // check category MIT owned
        auto address = info->mit.get_address();
        if (!blockchain.get_account_address(auth_.name, address)) {
            throw address_dismatch_account_exception{"Category MIT is not owned by account. " + category};
        }
    }

    // check to did
    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw address_invalid_exception{"invalid did parameter! " + to_did};
    }
    if (!blockchain.get_account_address(auth_.name, to_address)) {
        throw address_dismatch_account_exception{"target did does not match account. " + to_did};
    }

    // receiver
    std::vector<receiver_record> receiver;
    for (auto& pair : mit_map) {
        receiver.push_back(
            {
                to_address, pair.first, 0, 0, 0,
                utxo_attach_type::asset_mit, attachment(to_did, to_did)
            }
        );
    }

    auto helper = registering_mit(
                      *this, blockchain,
                      std::move(auth_.name), std::move(auth_.auth),
                      std::move(to_address),
                      std::move(category), std::move(mit_map),
                      std::move(receiver), argument_.fee,
                      is_create_category);

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

