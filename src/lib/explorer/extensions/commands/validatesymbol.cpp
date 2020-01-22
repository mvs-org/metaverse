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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/commands/validatesymbol.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::chain;

/************************ validatesymbol *************************/

console_result validatesymbol::invoke(Json::Value& jv_output, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    if (argument_.symbol.empty()) {
        throw asset_symbol_length_exception{"Symbol cannot be empty."};
    }

    auto& symbol = argument_.symbol;

    const std::string Available("available");
    const std::string Existed("existed");
    const std::string Forbidden("forbidden");
    const std::string No_Permission("no_permission");

    // check asset symbol:available/existed/forbidden/no_permission
    std::string key("asset_symbol");
    try
    {
        check_asset_symbol(symbol, true);

        // check asset exists
        if (blockchain.is_asset_exist(symbol, true)) {
            jv_output[key] = Existed;
        }
        else {
            auto&& domain = asset_cert::get_domain(symbol);
            if (blockchain.is_asset_cert_exist(domain, asset_cert_ns::domain)) {
                auto result_vec = blockchain.get_account_asset_certs(
                                      auth_.name, domain, asset_cert_ns::domain);
                if (nullptr == result_vec || result_vec->empty()) {
                    jv_output[key] = No_Permission;
                }
                else {
                    jv_output[key] = Available;
                }
            }
            else {
                jv_output[key] = Available;
            }
        }
    }
    catch (...)
    {
        jv_output[key] = Forbidden;
    }

    // check mit symbol:available/existed/forbidden
    key = "mit_symbol";
    try
    {
        check_mit_symbol(symbol, true);

        // check mit exists
        if (blockchain.is_asset_mit_exist(symbol)) {
            jv_output[key] = Existed;
        }
        else {
            jv_output[key] = Available;
        }
    }
    catch (...)
    {
        jv_output[key] = Forbidden;
    }

    // check did symbol:available/existed/forbidden
    key = "did_symbol";
    try
    {
        check_did_symbol(symbol, true);

        // check did exists
        if (blockchain.is_did_exist(symbol)) {
            jv_output[key] = Existed;
        }
        else {
            jv_output[key] = Available;
        }
    }
    catch (...)
    {
        jv_output[key] = Forbidden;
    }

    // check cert symbol:available/existed/forbidden/no_permission
    if (!option_.cert_type.empty()) {
        key = "cert_symbol";
        boost::to_lower(option_.cert_type);

        try
        {
            check_asset_symbol(symbol);

            auto certs_create = check_issue_cert(
                blockchain, auth_.name, symbol, option_.cert_type);
            if (certs_create != asset_cert_ns::none) {
                jv_output[key] = Available;
            }
            else {
                jv_output[key] = Forbidden;
            }
        }
        catch (const asset_cert_existed_exception&) {
            jv_output[key] = Existed;
        }
        catch (const asset_cert_notfound_exception&) {
            jv_output[key] = No_Permission;
        }
        catch (const asset_cert_notowned_exception&) {
            jv_output[key] = No_Permission;
        }
        catch (...)
        {
            jv_output[key] = Forbidden;
        }
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

