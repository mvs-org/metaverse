/**
 * Copyright (c) 2016-2017 mvs developers 
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

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin.hpp>
#include <metaverse/client.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/wallet/encodeattachtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
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

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

