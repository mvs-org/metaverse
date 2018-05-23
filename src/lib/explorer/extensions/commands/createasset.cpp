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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/createasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;
/************************ createasset *************************/

void validate(boost::any& v,
    const std::vector<std::string>& values, non_negative_uint64*, int)
{
    using namespace boost::program_options;
    validators::check_first_occurrence(v);

    std::string const& s = validators::get_single_string(values);
    if (s[0] == '-') {
        throw argument_legality_exception{"volume cannot be anegative number."};
    }
    v = boost::any(non_negative_uint64 { boost::lexical_cast<uint64_t>(s) } );
}

console_result createasset::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(option_.symbol);

    // check asset symbol
    check_asset_symbol(option_.symbol, true);

    // check did symbol
    auto issuer_did = option_.issuer;
    check_did_symbol(issuer_did);

    if (option_.description.length() > ASSET_DETAIL_DESCRIPTION_FIX_SIZE)
        throw asset_description_length_exception{"asset description length must be less than 64."};
    auto threshold = option_.secondaryissue_threshold;
    if ((threshold < -1) || (threshold > 100)) {
        throw asset_secondaryissue_threshold_exception{
            "secondaryissue threshold value error, it must be -1 or in the interval 0 to 100."};
    }

    if (option_.decimal_number > 19u)
        throw asset_amount_exception{"asset decimal number must less than 20."};
    if (option_.maximum_supply.volume == 0u)
        throw argument_legality_exception{"volume cannot be zero."};

    // check did exists
    if (!blockchain.is_did_exist(issuer_did)) {
        throw did_symbol_notfound_exception{
            "The did '" + issuer_did + "' does not exist on the blockchain, maybe you should issuedid first"};
    }

    // check did is owned by the account
    if (!blockchain.is_account_owned_did(auth_.name, issuer_did)) {
        throw did_symbol_notowned_exception{
            "The did '" + issuer_did + "' is not owned by " + auth_.name};
    }

    // check asset exists
    if (blockchain.is_asset_exist(option_.symbol, true))
        throw asset_symbol_existed_exception{"symbol is already used."};

    // local database asset check
    auto sh_asset = blockchain.get_account_unissued_asset(auth_.name, option_.symbol);
    if (sh_asset) {
        throw asset_symbol_duplicate_exception{option_.symbol
            + " already created, you can delete and recreate it."};
    }

    auto acc = std::make_shared<asset_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_maximum_supply(option_.maximum_supply.volume);
    acc->set_decimal_number(static_cast<uint8_t>(option_.decimal_number));
    acc->set_issuer(issuer_did);
    acc->set_description(option_.description);
    // use 127 to represent freely secondary issue, and 255 for its secondary issued status.
    acc->set_secondaryissue_threshold((threshold == -1) ?
        asset_detail::freely_secondaryissue_threshold : static_cast<uint8_t>(threshold));

    blockchain.store_account_asset(acc, auth_.name);

    Json::Value asset_data = config::json_helper(get_api_version()).prop_list(*acc, true);
    asset_data["status"] = "unissued";
    jv_output["asset"] = asset_data;

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

