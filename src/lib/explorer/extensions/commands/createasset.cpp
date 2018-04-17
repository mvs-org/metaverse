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

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;
/************************ createasset *************************/

void validate(boost::any& v,
              const std::vector<std::string>& values,
              non_negative_uint64*, int)
{
    using namespace boost::program_options;
    validators::check_first_occurrence(v);

    std::string const& s = validators::get_single_string(values);
    if (s[0] == '-') {
        throw argument_legality_exception{"volume must not be negative number."};
    }
    v = boost::any(non_negative_uint64 { boost::lexical_cast<uint64_t>(s) } );
}

console_result createasset::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check options
    if (option_.symbol.empty())
        throw asset_symbol_length_exception{"asset symbol can not be empty."};
    if (option_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    if (option_.description.length() > ASSET_DETAIL_DESCRIPTION_FIX_SIZE)
        throw asset_description_length_exception{"asset description length must be less than 64."};
    auto threshold = option_.secondaryissue_threshold;
    if ((threshold < -1) || (threshold > 100)) {
        throw asset_secondaryissue_threshold_exception{"secondaryissue threshold value error, is must be -1 or in range of 0 to 100."};
    }
    if (option_.decimal_number > 19u)
        throw asset_amount_exception{"asset decimal number must less than 20."};
    if (option_.maximum_supply.volume == 0u)
        throw argument_legality_exception{"volume must not be zero."};

    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);

    if(bc::wallet::symbol::is_sensitive(option_.symbol)) {
        throw asset_symbol_name_exception{"invalid symbol start with " + option_.symbol};
    }

    if(blockchain.is_asset_exist(option_.symbol)) 
        throw asset_symbol_existed_exception{"symbol is already used."};

    auto acc = std::make_shared<asset_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_maximum_supply(option_.maximum_supply.volume);
    acc->set_decimal_number(static_cast<uint8_t>(option_.decimal_number));
    acc->set_issuer(auth_.name);
    acc->set_description(option_.description);
    // use 127 to represent freely secondary issue, and 255 for its secondary issued status.
    acc->set_secondaryissue_threshold((threshold == -1) ? 127 : threshold);
    
    blockchain.store_account_asset(acc);

    auto& aroot = jv_output;
    Json::Value asset_data = config::json_helper(get_api_version()).prop_list(*acc, true);
    asset_data["status"] = "unissued";
    aroot["asset"] = asset_data;

    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin

