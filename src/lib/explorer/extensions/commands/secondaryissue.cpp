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
#include <metaverse/explorer/extensions/commands/secondaryissue.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ secondaryissue *************************/
console_result secondaryissue::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};

    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address))
        throw address_invalid_exception{"invalid did parameter! " + to_did};

    if (!blockchain.get_account_address(auth_.name, to_address))
        throw address_dismatch_account_exception{"target address does not match account. " + to_address};

    auto asset = blockchain.get_issued_asset(argument_.symbol);
    if (!asset) {
        throw asset_symbol_notfound_exception{"asset symbol is not exist in blockchain"};
    }

    auto from_did = asset->get_issuer();
    auto secondaryissue_threshold = asset->get_secondaryissue_threshold();
    if (!asset_detail::is_secondaryissue_legal(secondaryissue_threshold))
        throw asset_secondaryissue_threshold_exception{"asset is not allowed to secondary issue, or the threshold is illegal."};

    if (!blockchain.is_asset_cert_exist(argument_.symbol, asset_cert_ns::issue)) {
        throw asset_cert_exception{"no issue asset cert exist in blockchain for symbol " + argument_.symbol};
    }

    // if issue cert exists then check whether it belongs to the account.
    const auto match = [](const business_address_asset_cert& item) {
        return asset_cert::test_certs(item.certs.get_certs(), asset_cert_ns::issue);
    };

    auto certs_vec = blockchain.get_account_asset_certs(auth_.name, argument_.symbol);
    const auto it = std::find_if(certs_vec->begin(), certs_vec->end(), match);
    if (it == certs_vec->end()) {
        throw asset_cert_exception{"no issue asset cert owned for symbol " + argument_.symbol};
    }
    auto cert_address = it->address;
    auto cert_did = it->certs.get_owner_from_address(blockchain);

    auto total_volume = blockchain.get_asset_volume(argument_.symbol);
    if (total_volume > max_uint64 - argument_.volume)
        throw asset_amount_exception{"secondaryissue volume cannot exceed maximum value"};

    uint64_t asset_volume_of_threshold = 0;
    if (!asset_detail::is_secondaryissue_freely(secondaryissue_threshold)) {
        asset_volume_of_threshold = (uint64_t)(((double)total_volume) / 100 * secondaryissue_threshold);
    }

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, asset_volume_of_threshold,
            utxo_attach_type::asset_secondaryissue, attachment(from_did, to_did)},
        {cert_address, argument_.symbol, 0, 0, asset_cert_ns::issue,
            utxo_attach_type::asset_cert, attachment(cert_did, cert_did)}
    };

    auto issue_helper = secondary_issuing_asset(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        std::move(to_address), std::move(argument_.symbol),
        std::move(option_.attenuation_model_param),
        std::move(receiver), argument_.fee, argument_.volume);

    issue_helper.exec();

    // json output
    auto tx = issue_helper.get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

