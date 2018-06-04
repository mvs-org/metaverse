/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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

#include <cstdint>
#include <metaverse/client.hpp>
#include <metaverse/explorer/config/script.hpp>

using namespace bc::client;
using namespace bc::config;
using namespace bc::wallet;

namespace libbitcoin {
namespace explorer {
namespace config {

std::ostream& operator<<(std::ostream& out, char c) {
    out << std::to_string(c);
    return out;
}

std::ostream& operator<<(std::ostream& out, unsigned char c) {
    out << std::to_string(c);
    return out;
}

template <typename Value>
Json::Value& operator+=(Json::Value& a, const Value& b)
{
    std::ostringstream ss;
    ss << b;
    a = ss.str();
    return a;
}

template <typename Value>
std::string operator+(const Value& value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

Json::Value json_helper::prop_list(const header& header)
{
    const chain::header& block_header = header;

    Json::Value tree;

    tree["hash"] += hash256(block_header.hash());
    tree["merkle_tree_hash"] += hash256(block_header.merkle);
    tree["previous_block_hash"] += hash256(block_header.previous_block_hash);
    tree["bits"] += block_header.bits;
    tree["mixhash"] += block_header.mixhash;
    tree["nonce"] += block_header.nonce;

    if (version_ == 1) {
        tree["time_stamp"] += block_header.timestamp;
        tree["version"] += block_header.version;
        tree["number"] += block_header.number;
        tree["transaction_count"] += block_header.transaction_count;
    } else {
        tree["time_stamp"] = block_header.timestamp;
        tree["version"] = block_header.version;
        tree["number"] = block_header.number;
        tree["transaction_count"] = block_header.transaction_count;
    }

    return tree;
}
Json::Value json_helper::prop_tree(const header& header)
{
    Json::Value tree;
    tree["result"] = prop_list(header);
    return tree;
}
Json::Value json_helper::prop_tree(const std::vector<header>& headers, bool json)
{
    Json::Value tree;
    tree["headers"] = prop_tree_list("header", headers, json);
    return tree;
}

// transfers

Json::Value json_helper::prop_list(const chain::history& row)
{
    Json::Value tree;

    tree["received"] = Json::objectValue;
    // missing output implies output cut off by server's history threshold
    if (row.output.hash != null_hash)
    {
        tree["received"]["hash"] += hash256(row.output.hash);

        // zeroized received.height implies output unconfirmed (in mempool)
        if (row.output_height != 0)
            tree["received"]["height"] += row.output_height;

        if (version_ == 1) {
            tree["received"]["index"] += row.output.index;
        } else {
            tree["received"]["index"] = row.output.index;
        }
    }

    tree["spent"] = Json::objectValue;
    // missing input implies unspent
    if (row.spend.hash != null_hash)
    {
        tree["spent"]["hash"] += hash256(row.spend.hash);

        // zeroized input.height implies spend unconfirmed (in mempool)
        if (row.spend_height != 0)
            tree["spent"]["height"] += row.spend_height;

        if (version_ == 1) {
            tree["spent"]["index"] += row.spend.index;
        } else {
            tree["spent"]["index"] = row.spend.index;
        }
    }

    if (version_ == 1) {
        tree["value"] += row.value;
    } else {
        tree["value"] = row.value;
    }
    return tree;
}
Json::Value json_helper::prop_tree(const chain::history& row)
{
    Json::Value tree;
    tree["transfer"] = prop_list(row);
    return tree;
}
Json::Value json_helper::prop_tree(const chain::history::list& rows, bool json)
{
    Json::Value tree;
    tree["transfers"] = prop_tree_list("transfer", rows, json);
    return tree;
}

// balance

Json::Value json_helper::prop_list(const chain::history::list& rows,
    const payment_address& balance_address)
{
    Json::Value tree;
    uint64_t total_received = 0;
    uint64_t confirmed_balance = 0;
    uint64_t unspent_balance = 0;

    for (const auto& row: rows)
    {
        total_received += row.value;

        // spend unconfirmed (or no spend attempted)
        if (row.spend.hash == null_hash)
            unspent_balance += row.value;

        if (row.output_height != 0 &&
            (row.spend.hash == null_hash || row.spend_height == 0))
            confirmed_balance += row.value;
    }

    tree["address"] += balance_address;
    if (version_ == 1) {
        tree["confirmed"] += confirmed_balance;
        tree["received"] += total_received;
        tree["unspent"] += unspent_balance;
    } else {
        tree["confirmed"] = confirmed_balance;
        tree["received"] = total_received;
        tree["unspent"] = unspent_balance;
    }
    return tree;
}
Json::Value json_helper::prop_tree(const chain::history::list& rows,
    const payment_address& balance_address)
{
    Json::Value tree;
    tree["balance"] = prop_list(rows, balance_address);
    return tree;
}

// inputs

Json::Value json_helper::prop_list(const tx_input_type& tx_input)
{
    Json::Value tree;
    const auto script_address = payment_address::extract(tx_input.script);
    if (script_address)
        tree["address"] += script_address;

    tree["previous_output"] = Json::objectValue;
    tree["previous_output"]["hash"] += hash256(tx_input.previous_output.hash);
    if (version_ == 1 ) {
        tree["previous_output"]["index"] += tx_input.previous_output.index;
        tree["sequence"] += tx_input.sequence;
    } else {
        tree["previous_output"]["index"] = tx_input.previous_output.index;
        tree["sequence"] = tx_input.sequence;
    }
    tree["script"] += script(tx_input.script).to_string();
    return tree;
}
Json::Value json_helper::prop_tree(const tx_input_type& tx_input)
{
    Json::Value tree;
    tree["input"] = prop_list(tx_input);
    return tree;
}
Json::Value json_helper::prop_tree(const tx_input_type::list& tx_inputs, bool json)
{
    Json::Value tree;
    tree["inputs"] = prop_tree_list("input", tx_inputs, json);
    return tree;
}

Json::Value json_helper::prop_list(const input& input)
{
    const tx_input_type& tx_input = input;
    return prop_list(tx_input);
}
Json::Value json_helper::prop_tree(const input& input)
{
    Json::Value tree;
    tree["input"] = prop_list(input);
    return tree;
}
Json::Value json_helper::prop_tree(const std::vector<input>& inputs, bool json)
{
    const auto tx_inputs = cast<input, tx_input_type>(inputs);

    Json::Value tree;
    tree["inputs"] = prop_tree_list("input", tx_inputs, json);
    return tree;
}

// outputs

Json::Value json_helper::prop_list(const tx_output_type& tx_output)
{
    Json::Value tree;
    const auto address = payment_address::extract(tx_output.script);
    if (address)
        tree["address"] += address;

    tree["script"] += script(tx_output.script).to_string();
    uint64_t lock_height = 0;
    if(chain::operation::is_pay_key_hash_with_lock_height_pattern(tx_output.script.operations))
        lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(tx_output.script.operations);
    // TODO: this will eventually change due to privacy problems, see:
    // lists.dyne.org/lurker/message/20140812.214120.317490ae.en.html

    if (!address)
    {
        tree["stealth"] = Json::objectValue;
        uint32_t stealth_prefix;
        ec_compressed ephemeral_key;
        if (to_stealth_prefix(stealth_prefix, tx_output.script) &&
            extract_ephemeral_key(ephemeral_key, tx_output.script))
        {
            tree["stealth"]["prefix"] += stealth_prefix;
            tree["stealth"]["ephemeral_public_key"] += ec_public(ephemeral_key);
        }
    }

    if (version_ == 1) {
        tree["value"] += tx_output.value;
        tree["locked_height_range"] += lock_height;
    } else {
        tree["value"] = tx_output.value;
        tree["locked_height_range"] = lock_height;
    }

    if (chain::operation::is_pay_key_hash_with_attenuation_model_pattern(tx_output.script.operations)) {
        auto model_param = tx_output.get_attenuation_model_param();
        tree["attenuation_model_param"] = prop_attenuation_model_param(model_param);
    }

    tree["attachment"] = prop_list(const_cast<bc::chain::attachment&>(tx_output.attach_data));
    return tree;
}

Json::Value json_helper::prop_attenuation_model_param(const data_chunk& chunk)
{
    Json::Value tree;
    std::string param_str(chunk.begin(), chunk.end());
    const auto& kv_vec = bc::split(param_str, ";", true);
    std::vector<uint64_t> uc_vec, uq_vec;
    for (const auto& kv : kv_vec) {
        auto vec = bc::split(kv, "=", true);
        if (vec.size() == 2) {
            auto& key = vec[0];
            auto& value = vec[1];
            if (attenuation_model::is_multi_value_key(key)) {
                try {
                    if (key == "UC") {
                        auto&& str_vec = bc::split(value, ",", true);
                        for (auto& str : str_vec) {
                            uc_vec.push_back(std::stoull(str));
                        }
                    }
                    else if (key == "UQ") {
                        auto&& str_vec = bc::split(value, ",", true);
                        for (auto& str : str_vec) {
                            uq_vec.push_back(std::stoull(str));
                        }
                    }
                }
                catch (const std::exception& e) {
                    uc_vec.clear();
                    uq_vec.clear();
                    log::info("json_helper") << "invalid attenuation_model_param: " << param_str;
                }
            }
            else {
                uint64_t num = std::stoull(value);
                auto display_key = attenuation_model::get_name_of_key(key);
                tree[display_key] = num;
            }
        }
    }

    if (uc_vec.size() > 0 && uc_vec.size() == uq_vec.size()) {
        Json::Value nodes;

        for (size_t i = 0; i < uc_vec.size(); ++i) {
            Json::Value node;
            node["number"] = uc_vec[i];
            node["quantity"] = uq_vec[i];
            nodes.append(node);
        }

        tree["locked"] = nodes;
    }

    return tree;
}

Json::Value json_helper::prop_list(const tx_output_type& tx_output, uint32_t index)
{
    Json::Value tree;

    tree = prop_list(tx_output);

    if (version_ == 1) {
        tree["index"] += index;
    } else {
        tree["index"] = index;
    }

    return tree;
}

// is_secondaryissue has no meaning for asset quantity summary.
// don't add address info if show_address is not true.
Json::Value json_helper::prop_list(const bc::chain::asset_detail& detail_info,
        bool is_maximum_supply, bool show_address)
{
    Json::Value tree;
    tree["symbol"] = detail_info.get_symbol();
    tree["issuer"] = detail_info.get_issuer();
    if (show_address) {
        tree["address"] = detail_info.get_address();
    }
    tree["description"] = detail_info.get_description();

    const char* maximum_supply_or_quantity = is_maximum_supply ? "maximum_supply" : "quantity";

    if (version_ == 1) {
        tree[maximum_supply_or_quantity] += detail_info.get_maximum_supply();
        tree["decimal_number"] += detail_info.get_decimal_number();
        tree["secondaryissue_threshold"] += detail_info.get_secondaryissue_threshold();
        if (is_maximum_supply) {
            tree["is_secondaryissue"] = detail_info.is_asset_secondaryissue() ? "true" : "false";
        }
    } else {
        tree[maximum_supply_or_quantity] = detail_info.get_maximum_supply();
        tree["decimal_number"] = detail_info.get_decimal_number();
        tree["secondaryissue_threshold"] = detail_info.get_secondaryissue_threshold();
        if (is_maximum_supply) {
            tree["is_secondaryissue"] = detail_info.is_asset_secondaryissue();
        }
    }
    return tree;
}

// balance_info only "symbol" "address" "quantity" info included in it.
// issued_info include the other info.
// is_secondaryissue has no meaning for asset quantity summary.
// don't add address info if show_address is not true.
Json::Value json_helper::prop_list(const bc::chain::asset_balances& balance_info,
        const bc::chain::asset_detail& issued_info, bool show_address)
{
    Json::Value tree;
    tree["symbol"] = balance_info.symbol;
    if (show_address) {
        tree["address"] = balance_info.address;
    }

    tree["issuer"] = issued_info.get_issuer();
    tree["description"] = issued_info.get_description();

    if (version_ == 1) {
        tree["quantity"] += balance_info.unspent_asset;
        tree["locked_quantity"] += balance_info.locked_asset;

        tree["decimal_number"] += issued_info.get_decimal_number();
        tree["secondaryissue_threshold"] += issued_info.get_secondaryissue_threshold();
    } else {
        tree["quantity"] = balance_info.unspent_asset;
        tree["locked_quantity"] = balance_info.locked_asset;

        tree["decimal_number"] = issued_info.get_decimal_number();
        tree["secondaryissue_threshold"] = issued_info.get_secondaryissue_threshold();
    }
    return tree;
}

Json::Value json_helper::prop_list(const bc::chain::asset_transfer& trans_info, uint8_t decimal_number)
{
    Json::Value tree;
    tree["symbol"] = trans_info.get_symbol();

    if (version_ == 1) {
        tree["quantity"] += trans_info.get_quantity();
    } else {
        tree["quantity"] = trans_info.get_quantity();
    }

    if (decimal_number != max_uint8) {
        if (version_ == 1) {
            tree["decimal_number"] += decimal_number;
        } else {
            tree["decimal_number"] = decimal_number;
        }
    }
    return tree;
}

Json::Value json_helper::prop_list(const bc::chain::asset_cert& cert_info)
{
    Json::Value tree;
    tree["symbol"] = cert_info.get_symbol();
    tree["owner"] = cert_info.get_owner();
    tree["address"] = cert_info.get_address();
    tree["cert"] = cert_info.get_type_name();
    return tree;
}

Json::Value json_helper::prop_list(const bc::chain::asset_mit& mit_info, bool always_show_content)
{
    Json::Value tree;
    tree["symbol"] = mit_info.get_symbol();
    tree["address"] = mit_info.get_address();
    tree["status"] = mit_info.get_status_name();

    if (always_show_content || mit_info.is_register_status()) {
        tree["content"] = mit_info.get_content();
    }

    return tree;
}

Json::Value json_helper::prop_list(const bc::chain::asset_mit_info& mit_info)
{
    Json::Value tree;

    tree["height"] = mit_info.output_height;
    tree["time_stamp"] = mit_info.timestamp;
    tree["from_did"] = mit_info.from_did;
    tree["to_did"] = mit_info.to_did;

    tree["symbol"] = mit_info.mit.get_symbol();
    tree["address"] = mit_info.mit.get_address();
    tree["status"] = mit_info.mit.get_status_name();

    return tree;
}

Json::Value json_helper::prop_list(const bc::chain::account_multisig& acc_multisig)
{
    Json::Value tree, pubkeys;
    for (const auto& each : acc_multisig.get_cosigner_pubkeys()) {
        pubkeys.append(each);
    }

    if (version_ == 1) {
        tree["index"] += acc_multisig.get_index();
        tree["m"] += acc_multisig.get_m();
        tree["n"] += acc_multisig.get_n();
    }
    else {
        tree["index"] = acc_multisig.get_index();
        tree["m"] = acc_multisig.get_m();
        tree["n"] = acc_multisig.get_n();
    }

    if (version_ == 1 && pubkeys.isNull()) { //compatible for v1
        tree["public-keys"] = "";
    }
    else {
        tree["public-keys"] = pubkeys;
    }

    tree["address"] = acc_multisig.get_address();
    tree["self-publickey"] = acc_multisig.get_pub_key();
    tree["multisig-script"] = acc_multisig.get_multisig_script();
    tree["description"] = acc_multisig.get_description();
    return tree;
}

Json::Value json_helper::prop_list(bc::chain::attachment& attach_data)
{
    Json::Value tree;

    if (attach_data.get_type() == ETP_TYPE) {
        tree["type"] = "etp";
    }
    else if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<bc::chain::asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<bc::chain::asset_detail>(asset_info.get_data());
            tree = prop_list(detail_info, false);
            // add is_secondaryissue for asset-issue
            if (version_ == 1) {
                tree["is_secondaryissue"] = detail_info.is_asset_secondaryissue() ? "true" : "false";
            } else {
                tree["is_secondaryissue"] = detail_info.is_asset_secondaryissue();
            }
            tree["type"] = "asset-issue";
        }
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            auto trans_info = boost::get<bc::chain::asset_transfer>(asset_info.get_data());
            tree = prop_list(trans_info);
            tree["type"] = "asset-transfer";
        }
    }
    else if (attach_data.get_type() == ASSET_MIT_TYPE) {
        auto asset_info = boost::get<bc::chain::asset_mit>(attach_data.get_attach());
        tree = prop_list(asset_info);
        tree["type"] = "mit";
    }
    else if (attach_data.get_type() == ASSET_CERT_TYPE) {
        auto cert_info = boost::get<bc::chain::asset_cert>(attach_data.get_attach());
        tree = prop_list(cert_info);
        tree["type"] = "asset-cert";
    }
    else if (attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<bc::chain::did>(attach_data.get_attach());
        if (did_info.get_status() == DID_DETAIL_TYPE) {
            tree["type"] = "did-register";
            auto detail_info = boost::get<bc::chain::did_detail>(did_info.get_data());
            tree["symbol"] = detail_info.get_symbol();
            tree["address"] = detail_info.get_address();
        }
        if (did_info.get_status() == DID_TRANSFERABLE_TYPE) {
            tree["type"] = "did-transfer";
            auto detail_info = boost::get<bc::chain::did_detail>(did_info.get_data());
            tree["symbol"] = detail_info.get_symbol();
            tree["address"] = detail_info.get_address();
        }
    }
    else if (attach_data.get_type() == MESSAGE_TYPE) {
        tree["type"] = "message";
        auto msg_info = boost::get<bc::chain::blockchain_message>(attach_data.get_attach());
        tree["content"] = msg_info.get_content();
    }
    else {
        tree["type"] = "unknown business";
        BITCOIN_ASSERT(false);
    }

    if (attach_data.get_version() == DID_ATTACH_VERIFY_VERSION) {
        tree["from_did"] = attach_data.get_from_did();
        tree["to_did"] =  attach_data.get_to_did();
    }
    return tree;
}
Json::Value json_helper::prop_tree(const tx_output_type& tx_output)
{
    Json::Value tree;
    tree["output"] = prop_list(tx_output);
    return tree;
}
Json::Value json_helper::prop_tree(const tx_output_type::list& tx_outputs, bool json)
{

    Json::Value list;
    uint32_t index = 0;
    for (const auto& value: tx_outputs){
        list.append(prop_list(value, index));
        index++;
    }

    if (version_ == 1 && list.isNull()) { //compatible for v1
        list = "";
    }

    return list;
}
// points

Json::Value json_helper::prop_list(const chain::point& point)
{
    Json::Value tree;
    tree["hash"] += hash256(point.hash);
    if (version_ == 1) {
        tree["index"] += point.index;
    } else {
        tree["index"] = point.index;
    }
    return tree;
}

Json::Value json_helper::prop_tree(const chain::point::list& points, bool json)
{
    Json::Value tree;
    for (const auto& point: points)
        tree["points"] = prop_list(point);
    return tree;
}

Json::Value json_helper::prop_tree(const chain::points_info& points_info, bool json)
{
    Json::Value tree;
    tree["points"] = prop_tree_list("points", points_info.points, json);
    tree["change"] += points_info.change;
    return tree;
}

// transactions

Json::Value json_helper::prop_list(const transaction& transaction, bool json)
{
    const tx_type& tx = transaction;

    Json::Value tree;
    if (json) {
        tree["hash"] += hash256(tx.hash());
        tree["inputs"] = prop_tree_list("input", tx.inputs, json);
        tree["lock_time"] += tx.locktime;
        tree["outputs"] = prop_tree(tx.outputs, json); // only used for output to add new field "index"
        tree["version"] += tx.version;
        return tree;
    }else {
        std::ostringstream sout;
        sout << base16(tx.to_data());
        tree["raw"] = sout.str();
    }
    return tree;
}
Json::Value json_helper::prop_list(const transaction& transaction, uint64_t tx_height, bool json)
{
    const tx_type& tx = transaction;

    Json::Value tree;
    tree["hash"] += hash256(tx.hash());
    if (version_ == 1) {
        tree["height"] += tx_height;
    } else {
        tree["height"] = tx_height;
    }
    tree["inputs"] = prop_tree_list("input", tx.inputs, json);
    tree["lock_time"] += tx.locktime;
    tree["outputs"] = prop_tree(tx.outputs, json); // only used for output to add new field "index"
    tree["version"] += tx.version;
    return tree;
}

Json::Value json_helper::prop_tree(const transaction& transaction, bool json)
{
    Json::Value tree;
    tree["transaction"] = prop_list(transaction, json);
    return tree;
}
Json::Value json_helper::prop_tree(const std::vector<transaction>& transactions, bool json)
{
    Json::Value tree;
    tree["transactions"] =
        prop_tree_list_of_lists("transaction", transactions, json);
    return tree;
}

// wrapper

Json::Value json_helper::prop_list(const wallet::wrapped_data& wrapper)
{
    Json::Value tree;
    tree["checksum"] += wrapper.checksum;
    tree["payload"] += base16(wrapper.payload);
    tree["version"] += wrapper.version;
    return tree;
}
Json::Value json_helper::prop_tree(const wallet::wrapped_data& wrapper)
{
    Json::Value tree;
    tree["wrapper"] = prop_list(wrapper);
    return tree;
}

Json::Value json_helper::prop_list(const tx_type& tx, const hash_digest& block_hash,
    const payment_address& address, bool json)
{
    Json::Value tree;
    tree["block"] += hash256(block_hash);
    tree["address"] += address;
    tree["transaction"] = prop_list(tx, json);
    return tree;
}
Json::Value json_helper::prop_tree(const tx_type& tx, const hash_digest& block_hash,
    const payment_address& address, bool json)
{
    Json::Value tree;
    tree["watch_address"] = prop_list(tx, block_hash, address, json);
    return tree;
}

// stealth_address

Json::Value json_helper::prop_list(const stealth_address& stealth, bool json)
{
    // We don't serialize a "reuse key" value as this is strictly an
    // optimization for the purpose of serialization and otherwise complicates
    // understanding of what is actually otherwise very simple behavior.
    // So instead we emit the reused key as one of the spend keys.
    // This means that it is typical to see the same key in scan and spend.

    const auto spends = cast<ec_compressed, ec_public>(stealth.spend_keys());
    const auto spends_values = prop_value_list("public_key", spends, json);

    Json::Value tree;
    tree["encoded"] += stealth;
    tree["filter"] += stealth.filter();
    tree["scan_public_key"] += ec_public(stealth.scan_key());
    tree["signatures"] += stealth.signatures();
    tree["spends"] = spends_values;
    tree["version"] += stealth.version();
    return tree;
}
Json::Value json_helper::prop_tree(const stealth_address& stealth, bool json)
{
    Json::Value tree;
    tree["stealth_address"] = prop_list(stealth, json);
    return tree;
}

// stealth

Json::Value json_helper::prop_list(const chain::stealth& row)
{
    Json::Value tree;
    tree["ephemeral_public_key"] += ec_public(row.ephemeral_public_key);
    tree["public_key_hash"] += hash160(row.public_key_hash);
    tree["transaction_hash"] += hash256(row.transaction_hash);
    return tree;
}
Json::Value json_helper::prop_tree(const chain::stealth& row)
{
    Json::Value tree;
    tree["match"] = prop_list(row);
    return tree;
}

Json::Value json_helper::prop_tree(const chain::stealth::list& rows, bool json)
{
    Json::Value tree;
    tree["stealth"] = prop_tree_list("match", rows, json);
    return tree;
}

// metadata

Json::Value json_helper::prop_list(const hash_digest& hash, size_t height, size_t index)
{
    Json::Value tree;
    tree["hash"] += hash256(hash);
    if (version_ == 1) {
        tree["height"] += height;
        tree["index"] += index;
    } else {
        tree["height"] = static_cast<uint64_t>(height);
        tree["index"] = static_cast<uint32_t>(index);
    }
    return tree;
}
Json::Value json_helper::prop_tree(const hash_digest& hash, size_t height, size_t index)
{
    Json::Value tree;
    tree["metadata"] = prop_list(hash, height, index);
    return tree;
}

// settings

Json::Value json_helper::prop_tree(const settings_list& settings)
{
    Json::Value list;
    for (const auto& setting: settings)
        list[setting.first] = setting.second;

    Json::Value tree;
    tree["settings"] = list;
    return tree;
}

// uri

Json::Value json_helper::prop_tree(const bitcoin_uri& uri)
{
    Json::Value uri_props;

    if (!uri.address().empty())
        uri_props["address"] = uri.address();

    if (uri.amount() != 0)
        uri_props["amount"] += uri.amount();

    if (!uri.label().empty())
        uri_props["label"] = uri.label();

    if (!uri.message().empty())
        uri_props["message"] = uri.message();

    if (!uri.r().empty())
        uri_props["r"] = uri.r();

    uri_props["scheme"] = "bitcoin";

    Json::Value tree;
    tree["uri"] = uri_props;
    return tree;
}

//block


Json::Value json_helper::prop_tree(const block& block, bool json, bool tx_json)
{
    Json::Value tree;

    if (json) {
        tree["header"] = prop_tree(block.header);
        std::vector<transaction> txs;
        txs.resize(block.transactions.size());
        std::copy(block.transactions.begin(), block.transactions.end(), txs.begin());
        tree["txs"] = prop_tree(txs, tx_json);
    } else {
        std::ostringstream sout;
        sout << encode_base16(block.to_data());
        tree["raw"] = sout.str();
    }

    return tree;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
