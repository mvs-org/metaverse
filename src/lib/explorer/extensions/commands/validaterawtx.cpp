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
#include <metaverse/explorer/extensions/commands/validaterawtx.hpp>
#include <metaverse/explorer/extensions/commands/sendrawtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

console_result show_error(Json::Value& jv_output, std::string message){
    jv_output["valid"] = false;
    jv_output["message"] = message;
    return console_result::okay;
}

console_result validaterawtx::invoke(Json::Value& jv_output,
                                 libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    tx_type tx_ = argument_.transaction;

    uint64_t outputs_etp_val = tx_.total_output_value();
    uint64_t inputs_etp_val = 0;
    if (!blockchain.get_tx_inputs_etp_value(tx_, inputs_etp_val))
        return show_error(jv_output, "get transaction inputs etp value error!");

    // check raw tx fee range
    if (inputs_etp_val <= outputs_etp_val) {
        return show_error(jv_output, "no enough transaction fee");
    }
    base_transfer_common::check_fee_in_valid_range(inputs_etp_val - outputs_etp_val);

    code ec = blockchain.validate_transaction(tx_);
    if (ec.value() != error::success) {
        return show_error(jv_output, ec.message());
    }

    if (option_.decode) {
        jv_output["tx"] = config::json_helper(get_api_version()).prop_tree(tx_, true);
    }

    jv_output["valid"] = true;
    jv_output["hash"] = encode_hash(tx_.hash());

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

