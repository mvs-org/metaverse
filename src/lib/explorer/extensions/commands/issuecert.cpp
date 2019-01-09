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
#include <metaverse/explorer/extensions/commands/issuecert.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result issuecert::invoke (Json::Value& jv_output,
                                  libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    blockchain.uppercase_symbol(argument_.symbol);
    boost::to_lower(argument_.cert);

    // check asset symbol
    check_asset_symbol(argument_.symbol);

    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw address_invalid_exception{"invalid did parameter! " + to_did};
    }

    if (!blockchain.get_account_address(auth_.name, to_address)) {
        throw address_dismatch_account_exception{"target did does not match account. " + to_did};
    }

    // check asset cert types
    auto certs_create = check_issue_cert(blockchain, auth_.name, argument_.symbol, argument_.cert);

    // update cert symbol
    std::string cert_symbol = argument_.symbol;
    std::string primary_symbol;

    if (certs_create == asset_cert_ns::witness) {
        uint32_t pri = 0, sec = 0;
        get_secondary_witness_cert_index(blockchain, pri, sec);

        if (pri == 0 || sec == 0) {
            throw asset_cert_secondary_witness_full_exception("secondary witness cert reaches the maximum.");
        }

        auto fmt = boost::format("%1%%2%") % witness_cert_prefix % pri;
        primary_symbol = fmt.str();

        fmt = boost::format("%1%.%2%.%3%") % primary_symbol % sec % argument_.symbol;
        cert_symbol = fmt.str();
        check_asset_symbol(cert_symbol);

        log::info("base_helper") << " issue_cert witness: primary cert: " << primary_symbol
            << ", secondary cert: " << cert_symbol;

#ifdef MVS_DEBUG
        auto pri_idx = asset_cert::get_primary_witness_index(cert_symbol);
        auto sec_idx = asset_cert::get_secondary_witness_index(cert_symbol);
        assert(pri_idx == pri);
        assert(sec_idx == sec);
#endif
    }

    // receiver
    //
    // init cert created
    std::vector<receiver_record> receiver{
        {
            to_address, cert_symbol, 0, 0,
            certs_create, utxo_attach_type::asset_cert_issue,
            attachment("", to_did)
        }
    };

    // append cert required
    if (certs_create == asset_cert_ns::naming) {
        auto&& domain = asset_cert::get_domain(cert_symbol);
        receiver.push_back({
            to_address, domain, 0, 0,
            asset_cert_ns::domain, utxo_attach_type::asset_cert,
            attachment("", to_did)
        });
    }
    else if (certs_create == asset_cert_ns::witness) {
        receiver.push_back({
            to_address, primary_symbol, 0, 0,
            asset_cert_ns::witness, utxo_attach_type::asset_cert,
            attachment("", to_did)
        });
    }

    // append memo
    if (!option_.memo.empty()) {
        check_message(option_.memo);

        receiver.push_back({
            to_address, "", 0, 0, utxo_attach_type::message,
            attachment(0, 0, chain::blockchain_message(option_.memo))
        });
    }

    auto helper = issuing_asset_cert(
                      *this, blockchain,
                      std::move(auth_.name), std::move(auth_.auth),
                      std::move(to_address), std::move(cert_symbol),
                      std::move(receiver), argument_.fee);

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

void issuecert::get_secondary_witness_cert_index(
    bc::blockchain::block_chain_impl& blockchain, uint32_t& pri, uint32_t& sec) const
{
    pri = 0;
    sec = 0;

    // get owned primary witness cert
    auto account_certs = blockchain.get_account_asset_certs(auth_.name, "", asset_cert_ns::witness);
    std::vector<uint32_t> pri_indexes;
    for (auto& bus_cert : *account_certs) {
        auto& cert = bus_cert.certs;
        if (cert.is_primary_witness()) {
            auto index = asset_cert::get_primary_witness_index(cert.get_symbol());
            pri_indexes.push_back(index);
        }
    }

    if (pri_indexes.empty()) {
        throw asset_cert_notowned_exception("no primary witness cert owned by " + auth_.name);
    }

    std::map<uint32_t, std::set<uint32_t>> pri_sec_map;
    auto issued_certs = blockchain.get_issued_asset_certs("", asset_cert_ns::witness);
    for (auto& cert : *issued_certs) {
        if (cert.is_secondary_witness()) {
            auto pri_index = asset_cert::get_primary_witness_index(cert.get_symbol());
            // not owned
            if (std::find(pri_indexes.begin(), pri_indexes.end(), pri_index) == pri_indexes.end()) {
                continue;
            }

            // record secondary witness
            auto sec_index = asset_cert::get_secondary_witness_index(cert.get_symbol());
            auto iter = pri_sec_map.find(pri_index);
            if (iter == pri_sec_map.end()) {
                std::set<uint32_t> set;
                set.insert(sec_index);
                pri_sec_map[pri_index] = set;
            }
            else {
                auto& set = iter->second;
                set.insert(sec_index);
            }
        }
    }

    // calculate primary and secondary index of new secondary cert.
    for (auto iter : pri_sec_map) {
        auto pri_index = iter.first;
        auto& set = iter.second;
        if (set.size() < witness_cert_count) {
            pri = pri_index;
            sec = set.size() + 1;
            break;
        }
        else {
            auto it = std::find(pri_indexes.begin(), pri_indexes.end(), pri_index);
            if (it != pri_indexes.end()) {
                pri_indexes.erase(it);
            }
        }
    }

    if (pri == 0 && !pri_indexes.empty()) {
        pri = pri_indexes[0];
        sec = 1;
    }
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

