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

#include <metaverse/macros_define.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/consensus/libdevcore/SHA3.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <regex>

namespace libbitcoin {
namespace explorer {
namespace commands {

using bc::blockchain::validate_transaction;
using namespace chain;
using namespace std;

utxo_attach_type get_utxo_attach_type(const chain::output& output_)
{
    auto& output = const_cast<chain::output&>(output_);
    if (output.is_etp()) {
        return utxo_attach_type::etp;
    }
    if (output.is_asset_transfer()) {
        return utxo_attach_type::asset_transfer;
    }
    if (output.is_asset_issue()) {
        return utxo_attach_type::asset_issue;
    }
    if (output.is_asset_secondaryissue()) {
        return utxo_attach_type::asset_secondaryissue;
    }
    if (output.is_asset_cert()) {
        return utxo_attach_type::asset_cert;
    }
    if (output.is_asset_mit()) {
        return utxo_attach_type::asset_mit;
    }
    if (output.is_did_register()) {
        return utxo_attach_type::did_register;
    }
    if (output.is_did_transfer()) {
        return utxo_attach_type::did_transfer;
    }
    if (output.is_message()) {
        return utxo_attach_type::message;
    }
    if (output.is_etp_award()) {
        throw std::logic_error("get_utxo_attach_type : Unexpected etp_award type.");
    }
    throw std::logic_error("get_utxo_attach_type : Unkown output type "
            + std::to_string(output.attach_data.get_type()));
}

//Check if address is checksum of ETH address
bool is_ETH_Address(const string& address)
{
    // regex checking
    {
        sregex_iterator end;

        // check if it has the basic requirements of an address
        static const regex reg_common("^0x[0-9a-fA-F]{40}$");
        sregex_iterator it(address.begin(), address.end(), reg_common);
        if (it == end) {
            return false;
        }

        // If it's all small caps, return true
        static const regex reg_alllower("^0x[0-9a-f]{40}$");
        sregex_iterator it1(address.begin(), address.end(), reg_alllower);
        if (it1 != end) {
            return true;
        }

        // If it's all caps, return true
        static const regex reg_allupper("^0x[0-9A-F]{40}$");
        sregex_iterator it2(address.begin(), address.end(), reg_allupper);
        if (it2 != end) {
            return true;
        }
    }

    // Otherwise check each case
    auto addr = address.substr(2); // get rid of prefix "0x"
    auto address_hash = bc::sha3(boost::to_lower_copy(addr)).hex();

    for (size_t i = 0; i < addr.size(); ++i) {
        auto c = addr[i];
        if (std::isdigit(c)) {
            continue;
        }
        // the nth letter should be uppercase if the nth digit of casemap is 1 (89abcdef)
        bool is_less_than_8 = (address_hash[i] >= '0' && address_hash[i] < '8');
        if ((is_less_than_8 && !std::islower(c)) ||
                (!is_less_than_8 && !std::isupper(c))) {
            return false;
        }
    }

    return true;
}

void check_did_symbol(const std::string& symbol, bool check_sensitive)
{
    if (!chain::output::is_valid_did_symbol(symbol, check_sensitive)) {
        throw did_symbol_name_exception{"Did symbol " + symbol + " is not valid."};
    }

    if (check_sensitive) {
        if (boost::iequals(symbol, "BLACKHOLE")) {
            throw did_symbol_name_exception{"Did symbol cannot be blackhole."};
        }
    }
}

void check_asset_symbol(const std::string& symbol, bool check_sensitive)
{
    if (symbol.empty()) {
        throw asset_symbol_length_exception{"Asset symbol cannot be empty."};
    }

    if (symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE) {
        throw asset_symbol_length_exception{"Asset symbol length must be less than "
            + std::to_string(ASSET_DETAIL_SYMBOL_FIX_SIZE) + "."};
    }

    if (check_sensitive) {
        if (bc::wallet::symbol::is_sensitive(symbol)) {
            throw asset_symbol_name_exception{"Asset symbol " + symbol + " is forbidden."};
        }
    }
}

void check_mit_symbol(const std::string& symbol, bool check_sensitive)
{
    if (symbol.empty()) {
        throw asset_symbol_length_exception{"MIT symbol cannot be empty."};
    }

    if (symbol.length() > ASSET_MIT_SYMBOL_FIX_SIZE) {
        throw asset_symbol_length_exception{"MIT symbol length must be less than "
            + std::to_string(ASSET_MIT_SYMBOL_FIX_SIZE) + "."};
    }

    // char check
    for (const auto& i : symbol) {
        if (!(std::isalnum(i) || i == '.'|| i == '@' || i == '_' || i == '-'))
            throw asset_symbol_name_exception(
                "MIT symbol " + symbol + " has invalid character.");
    }

    if (check_sensitive) {
        auto upper = boost::to_upper_copy(symbol);
        if (bc::wallet::symbol::is_sensitive(upper)) {
            throw asset_symbol_name_exception{"MIT symbol " + symbol + " is forbidden."};
        }
    }
}

void check_message(const std::string& message, bool check_sensitive)
{
    if (!message.empty() && message.size() >= 0xfd/* 253, see serializer.ipp */) {
        throw argument_size_invalid_exception{"message length out of bounds."};
    }
}

void check_mining_subsidy_param(const std::string& param)
{
    if (param.empty()) {
        return;
    }

    auto parameters = asset_cert::parse_mining_subsidy_param(param);
    if (parameters == nullptr) {
        throw asset_mining_subsidy_parameter_exception{"invalid parameters: " + param};
    }
}

template <typename ElemT>
struct HexTo {
    ElemT value;
    operator ElemT() const {return value;}
    friend std::istream& operator>>(std::istream& in, HexTo& out) {
        in >> std::hex >> out.value;
        return in;
    }
};

asset_cert_type check_cert_type_name(const string& cert_name, bool all)
{
    // check asset cert types
    auto certs_create = asset_cert_ns::none;
    std::map <std::string, asset_cert_type> cert_map = {
        {"naming",      asset_cert_ns::naming},
        {"marriage",    asset_cert_ns::marriage},
        {"kyc",         asset_cert_ns::kyc}
    };

    if (all) {
        cert_map["issue"] = asset_cert_ns::issue;
        cert_map["domain"] = asset_cert_ns::domain;
    }

    auto iter = cert_map.find(cert_name);
    if (iter != cert_map.end()) {
        certs_create = iter->second;
    }
    else {
        try {
            if (cert_name.compare(0, 2, "0x") == 0) {
                certs_create = boost::lexical_cast<HexTo<asset_cert_type>>(cert_name.c_str());
            }
            else {
                certs_create = boost::lexical_cast<asset_cert_type>(cert_name.c_str());
            }

            if (certs_create < asset_cert_ns::custom) {
                throw asset_cert_exception("invalid asset cert type " + cert_name);
            }
        }
        catch(boost::bad_lexical_cast const&) {
            throw asset_cert_exception("invalid asset cert type " + cert_name);
        }
    }
    return certs_create;
}

asset_cert_type check_issue_cert(bc::blockchain::block_chain_impl& blockchain,
    const string& account, const string& symbol, const string& cert_name)
{
    auto certs_create = check_cert_type_name(cert_name);

    // check domain naming cert not exist.
    if (blockchain.is_asset_cert_exist(symbol, certs_create)) {
        throw asset_cert_existed_exception(
            "cert '" + symbol + "' with type '" + cert_name + "' already exists on the blockchain!");
    }

    if (certs_create == asset_cert_ns::naming) {
        // check symbol is valid.
        auto pos = symbol.find(".");
        if (pos == std::string::npos) {
            throw asset_symbol_name_exception("invalid naming cert symbol " + symbol
                + ", it should contain a dot '.'");
        }

        auto&& domain = asset_cert::get_domain(symbol);
        if (!asset_cert::is_valid_domain(domain)) {
            throw asset_symbol_name_exception("invalid naming cert symbol " + symbol
                + ", it should contain a valid domain!");
        }

        // check asset not exist.
        if (blockchain.is_asset_exist(symbol, false)) {
            throw asset_symbol_existed_exception(
                "asset symbol '" + symbol + "' already exists on the blockchain!");
        }

        // check domain cert belong to this account.
        bool exist = blockchain.is_asset_cert_exist(domain, asset_cert_ns::domain);
        if (!exist) {
            throw asset_cert_notfound_exception("no domain cert '" + domain + "' found!");
        }

        auto cert = blockchain.get_account_asset_cert(account, domain, asset_cert_ns::domain);
        if (!cert) {
            throw asset_cert_notowned_exception("no domain cert '" + domain + "' owned by " + account);
        }
    }

    else if (certs_create == asset_cert_ns::mining) {
        log::info("base_helper") << " check_issue_cert mining: " << cert_name;

        // check asset exist.
        if (!blockchain.is_asset_exist(symbol, false)) {
            throw asset_symbol_existed_exception(
                "asset symbol '" + symbol + "' does not exist on the blockchain!");
        }

        // check domain cert belong to this account.
        bool exist = blockchain.is_asset_cert_exist(symbol, asset_cert_ns::issue);
        if (!exist) {
            throw asset_cert_notfound_exception("no issue cert '" + symbol + "' found!");
        }

        auto cert = blockchain.get_account_asset_cert(account, symbol, asset_cert_ns::issue);
        if (!cert) {
            throw asset_cert_notowned_exception("no issue cert '" + symbol + "' owned by " + account);
        }
    }

    return certs_create;
}

std::string get_address(const std::string& did_or_address,
    bc::blockchain::block_chain_impl& blockchain)
{
    std::string address;
    if (!did_or_address.empty()) {
        if (blockchain.is_valid_address(did_or_address)) {
            address = did_or_address;
        }
        else {
            address = get_address_from_did(did_or_address, blockchain);
        }
    }
    return address;
}

std::string get_address(const std::string& did_or_address,
    attachment& attach, bool is_from,
    bc::blockchain::block_chain_impl& blockchain)
{
    std::string address;
    if (blockchain.is_valid_address(did_or_address)) {
        address = did_or_address;
    }
    else {
        address = get_address_from_did(did_or_address, blockchain);
        if (is_from) {
            attach.set_from_did(did_or_address);
        }
        else {
            attach.set_to_did(did_or_address);
        }
        attach.set_version(DID_ATTACH_VERIFY_VERSION);
    }
    return address;
}

std::string get_address_from_did(const std::string& did,
    bc::blockchain::block_chain_impl& blockchain)
{
    check_did_symbol(did);

    auto diddetail = blockchain.get_registered_did(did);
    if (!diddetail) {
        throw did_symbol_notfound_exception{"did " + did + " does not exist on the blockchain"};
    }
    return diddetail->get_address();
}

std::string get_random_payment_address(
    std::shared_ptr<account_address::list> sp_addresses,
    bc::blockchain::block_chain_impl& blockchain)
{
    if (sp_addresses && !sp_addresses->empty()) {
        // first, let test 10 times of random
        for (auto i = 0; i < 10; ++i) {
            auto random = bc::pseudo_random();
            auto index = random % sp_addresses->size();
            auto addr = sp_addresses->at(index).get_address();
            if (blockchain.is_payment_address(addr)) {
                return addr;
            }
        }
        // then, real bad luck, lets filter only the payment address
        account_address::list filtered_addresses;
        std::copy_if(sp_addresses->begin(), sp_addresses->end(),
            std::back_inserter(filtered_addresses),
            [&blockchain](const auto& each){
               return blockchain.is_payment_address(each.get_address());
        });

        if (!filtered_addresses.empty()) {
            auto random = bc::pseudo_random();
            auto index = random % filtered_addresses.size();
            return filtered_addresses.at(index).get_address();
        }
    }
    return "";
}

void sync_fetch_asset_cert_balance(const std::string& address, const string& symbol,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<asset_cert::list> sh_vec,
    asset_cert_type cert_type)
{
    chain::transaction tx_temp;
    uint64_t tx_height;

    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));
    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(tx_temp, tx_height, row.output.hash))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address) {
                continue;
            }
            if (output.is_asset_cert())
            {
                auto asset_cert = output.get_asset_cert();
                if (!symbol.empty() && symbol != asset_cert.get_symbol()) {
                    continue;
                }
                if (cert_type != asset_cert_ns::none && cert_type != asset_cert.get_type()) {
                    continue;
                }

                sh_vec->push_back(std::move(asset_cert));
            }
        }
    }
}

void sync_fetch_asset_balance(const std::string& address, bool sum_all,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<asset_balances::list> sh_asset_vec)
{
    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(tx_temp, tx_height, row.output.hash))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address) {
                continue;
            }
            if (output.is_asset())
            {
                const auto& symbol = output.get_asset_symbol();
                if (bc::wallet::symbol::is_forbidden(symbol)) {
                    // swallow forbidden symbol
                    continue;
                }

                auto match = [sum_all, &symbol, &address](const asset_balances& elem) {
                    return (symbol == elem.symbol) && (sum_all || (address == elem.address));
                };
                auto iter = std::find_if(sh_asset_vec->begin(), sh_asset_vec->end(), match);

                auto asset_amount = output.get_asset_amount();
                uint64_t locked_amount = 0;
                if (asset_amount
                    && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    const auto& attenuation_model_param = output.get_attenuation_model_param();
                    auto diff_height = row.output_height
                        ? blockchain.calc_number_of_blocks(row.output_height, height)
                        : 0;
                    auto available_amount = attenuation_model::get_available_asset_amount(
                            asset_amount, diff_height, attenuation_model_param);
                    locked_amount = asset_amount - available_amount;
                }
                else if (asset_amount
                    && chain::operation::is_pay_key_hash_with_sequence_lock_pattern(output.script.operations)) {
                    uint64_t lock_sequence = chain::operation::
                        get_lock_sequence_from_pay_key_hash_with_sequence_lock(output.script.operations);
                    // use any kind of blocks
                    if (row.output_height + lock_sequence > height) {
                        // utxo already in block but is locked with sequence and not mature
                        locked_amount = asset_amount;
                    }
                }

                if (iter == sh_asset_vec->end()) { // new item
                    sh_asset_vec->push_back({symbol, address, asset_amount, locked_amount});
                }
                else { // exist just add amount
                    iter->unspent_asset += asset_amount;
                    iter->locked_asset += locked_amount;
                }
            }
        }
    }
}

void sync_fetch_locked_balance(const std::string& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<locked_balance::list> sh_vec,
    const std::string& asset_symbol,
    uint64_t expiration)
{
    const bool is_asset = !asset_symbol.empty();
    if (is_asset && wallet::symbol::is_forbidden(asset_symbol)) {
        return;
    }

    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));

    chain::transaction tx_temp;
    uint64_t tx_height = 0;

    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
            && blockchain.get_transaction(tx_temp, tx_height, row.output.hash))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);

            if (is_asset != output.is_asset()) {
                continue;
            }

            if (is_asset && asset_symbol != output.get_asset_symbol()) {
                continue;
            }

            if (!operation::is_pay_key_hash_with_sequence_lock_pattern(output.script.operations)) {
                continue;
            }

            uint64_t lock_sequence = chain::operation::
                get_lock_sequence_from_pay_key_hash_with_sequence_lock(output.script.operations);
            // use any kind of blocks
            if ((tx_height + lock_sequence <= height) ||
                (expiration > height && tx_height + lock_sequence <= expiration)) {
                continue;
            }

            uint64_t locked_value = is_asset ? output.get_asset_amount() : row.value;
            if (locked_value == 0) {
                continue;
            }

            uint64_t locked_height = lock_sequence;
            uint64_t expiration_height = tx_height + lock_sequence;
            locked_balance locked{address, locked_value, locked_height, expiration_height};
            sh_vec->emplace_back(locked);
        }
    }
}

void sync_fetch_asset_deposited_balance(const std::string& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<asset_deposited_balance::list> sh_asset_vec)
{
    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
            && blockchain.get_transaction(tx_temp, tx_height, row.output.hash))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
            if (output.is_asset())
            {
                if (!operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    continue;
                }

                auto asset_amount = output.get_asset_amount();
                if (asset_amount == 0) {
                    continue;
                }

                const auto& symbol = output.get_asset_symbol();
                if (bc::wallet::symbol::is_forbidden(symbol)) {
                    // swallow forbidden symbol
                    continue;
                }

                const auto& model_param = output.get_attenuation_model_param();
                auto diff_height = row.output_height
                    ? blockchain.calc_number_of_blocks(row.output_height, height)
                    : 0;
                auto available_amount = attenuation_model::get_available_asset_amount(
                        asset_amount, diff_height, model_param);
                uint64_t locked_amount = asset_amount - available_amount;
                if (locked_amount == 0) {
                    continue;
                }

                asset_deposited_balance deposited(
                    symbol, address, encode_hash(row.output.hash), row.output_height);
                deposited.unspent_asset = asset_amount;
                deposited.locked_asset = locked_amount;
                deposited.model_param = std::string(model_param.begin(), model_param.end());
                sh_asset_vec->push_back(deposited);
            }
        }
    }
}

void sync_unspend_output(bc::blockchain::block_chain_impl& blockchain, const input_point& input,
 std::shared_ptr<output_point::list>& output_list,  base_transfer_common::filter filter)
{
    auto is_filter = [filter](const output & output_){
        if (((filter & base_transfer_common::FILTER_ETP) && output_.is_etp())
        || ( (filter & base_transfer_common::FILTER_ASSET) && output_.is_asset())
        || ( (filter & base_transfer_common::FILTER_IDENTIFIABLE_ASSET) && output_.is_asset_mit())
        || ( (filter & base_transfer_common::FILTER_ASSETCERT) && output_.is_asset_cert())
        || ( (filter & base_transfer_common::FILTER_DID) && output_.is_did())){
            return true;
        }
        return false;
    };

    std::shared_ptr<chain::transaction> tx = blockchain.get_spends_output(input);
    uint64_t tx_height;
    chain::transaction tx_temp;
    if (tx == nullptr && blockchain.get_transaction(tx_temp, tx_height, input.hash))
    {
        const auto& output = tx_temp.outputs.at(input.index);

        if (is_filter(output)){
            output_list->emplace_back(input);
        }
    }
    else if (tx != nullptr)
    {
        for (uint32_t i = 0; i < tx->outputs.size(); i++)
        {
            const auto& output = tx->outputs.at(i);
            if (is_filter(output)){
                input_point input_ = {tx->hash(), i};
                sync_unspend_output(blockchain, input_, output_list, filter);
            }
        }
    }
}

auto get_asset_unspend_utxo(const std::string& symbol,
 bc::blockchain::block_chain_impl& blockchain) -> std::shared_ptr<output_point::list>
{
    auto blockchain_assets = blockchain.get_asset_register_output(symbol);
    if (blockchain_assets == nullptr || blockchain_assets->empty()){
        throw asset_symbol_existed_exception(std::string("asset symbol[") +symbol + "]does not exist!");
    }

    std::shared_ptr<output_point::list> output_list = std::make_shared<output_point::list>();
    for (auto asset : *blockchain_assets)
    {
        auto out_point = asset.get_tx_point();
        sync_unspend_output(blockchain, out_point, output_list, base_transfer_common::FILTER_ASSET);
    }
    if(!output_list->empty()){
        std::sort(output_list->begin(), output_list->end());
        output_list->erase(std::unique(output_list->begin(), output_list->end()), output_list->end());
    }
    return output_list;
}

auto sync_fetch_asset_deposited_view(const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain)
     -> std::shared_ptr<asset_deposited_balance::list>
{
    std::shared_ptr<output_point::list> output_list = get_asset_unspend_utxo(symbol, blockchain);
    std::shared_ptr<asset_deposited_balance::list> sh_asset_vec = std::make_shared<asset_deposited_balance::list>();

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto &out : *output_list)
    {
        // spend unconfirmed (or no spend attempted)
        if (blockchain.get_transaction(tx_temp, tx_height, out.hash))
        {
            BITCOIN_ASSERT(out.index < tx_temp.outputs.size());
            const auto &output = tx_temp.outputs.at(out.index);
            if (output.is_asset())
            {
                std::string address = output.get_script_address();

                const auto &symbol = output.get_asset_symbol();
                if (output.get_asset_symbol() != symbol ||
                    bc::wallet::symbol::is_forbidden(symbol))
                {
                    // swallow forbidden symbol
                    continue;
                }

                if (!operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    continue;
                }

                auto asset_amount = output.get_asset_amount();
                if (asset_amount == 0)  {
                    continue;
                }

                const auto &model_param = output.get_attenuation_model_param();
                auto diff_height = tx_height
                    ? blockchain.calc_number_of_blocks(tx_height, height)
                    : 0;
                auto available_amount = attenuation_model::get_available_asset_amount(
                    asset_amount, diff_height, model_param);
                uint64_t locked_amount = asset_amount - available_amount;
                if (locked_amount == 0) {
                    continue;
                }

                asset_deposited_balance deposited(
                    symbol, address, encode_hash(out.hash), tx_height);
                deposited.unspent_asset = asset_amount;
                deposited.locked_asset = locked_amount;
                deposited.model_param = std::string(model_param.begin(), model_param.end());
                sh_asset_vec->emplace_back(deposited);
            }
        }
    }

    return sh_asset_vec;
}

auto sync_fetch_asset_view(const std::string& symbol,
    bc::blockchain::block_chain_impl& blockchain)
     -> std::shared_ptr<asset_balances::list>
{
    std::shared_ptr<output_point::list> output_list = get_asset_unspend_utxo(symbol, blockchain);
    std::shared_ptr<asset_balances::list> sh_asset_vec = std::make_shared<asset_balances::list>();

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto &out : *output_list)
    {
        // spend unconfirmed (or no spend attempted)
        if (blockchain.get_transaction(tx_temp, tx_height, out.hash))
        {
            BITCOIN_ASSERT(out.index < tx_temp.outputs.size());
            const auto &output = tx_temp.outputs.at(out.index);
            if (output.is_asset())
            {
                std::string address = output.get_script_address();

                const auto &symbol = output.get_asset_symbol();
                if (output.get_asset_symbol() != symbol ||
                    bc::wallet::symbol::is_forbidden(symbol))
                {
                    // swallow forbidden symbol
                    continue;
                }

                auto asset_amount = output.get_asset_amount();
                uint64_t locked_amount = 0;
                if (asset_amount
                    && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
                    const auto& attenuation_model_param = output.get_attenuation_model_param();
                    auto diff_height = tx_height
                        ? blockchain.calc_number_of_blocks(tx_height, height)
                        : 0;
                    auto available_amount = attenuation_model::get_available_asset_amount(
                            asset_amount, diff_height, attenuation_model_param);
                    locked_amount = asset_amount - available_amount;
                }
                else if (asset_amount
                    && chain::operation::is_pay_key_hash_with_sequence_lock_pattern(output.script.operations)) {
                    uint64_t lock_sequence = chain::operation::
                        get_lock_sequence_from_pay_key_hash_with_sequence_lock(output.script.operations);
                    // use any kind of blocks
                    if (tx_height + lock_sequence > height) {
                        // utxo already in block but is locked with sequence and not mature
                        locked_amount = asset_amount;
                    }
                }

                sh_asset_vec->push_back({symbol, address, asset_amount, locked_amount});
            }
        }
    }

    return sh_asset_vec;
}

static uint32_t get_domain_cert_count(bc::blockchain::block_chain_impl& blockchain,
    const std::string& account_name)
{
    auto pvaddr = blockchain.get_account_addresses(account_name);
    if (!pvaddr) {
        return 0;
    }

    auto sh_vec = std::make_shared<asset_cert::list>();
    for (auto& each : *pvaddr){
        sync_fetch_asset_cert_balance(each.get_address(), "", blockchain, sh_vec, asset_cert_ns::domain);
    }

    return sh_vec->size();
}

void sync_fetch_deposited_balance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<deposited_balance::list> sh_vec)
{
    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    auto&& address_str = address.encoded();
    auto&& rows = blockchain.get_address_history(address, false);
    for (auto& row: rows) {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
            && blockchain.get_transaction(tx_temp, tx_height, row.output.hash)) {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            auto output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address.encoded()) {
                continue;
            }

            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                // deposit utxo in block
                uint64_t deposit_height = chain::operation::
                    get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                auto deposited_height = blockchain.calc_number_of_blocks(row.output_height, height);

                if (deposit_height > deposited_height) {
                    auto&& output_hash = encode_hash(row.output.hash);
                    auto&& tx_hash = encode_hash(tx_temp.hash());
                    const auto match = [&tx_hash](const deposited_balance& balance) {
                        return balance.tx_hash == tx_hash;
                    };
                    auto iter = std::find_if(sh_vec->begin(), sh_vec->end(), match);
                    if (iter != sh_vec->end()) {
                        if (output_hash == tx_hash) {
                            iter->balance = row.value;
                        }
                        else {
                            iter->bonus = row.value;
                            iter->bonus_hash = output_hash;
                        }
                    }
                    else {
                        uint64_t expiration_height =
                            blockchain.get_expiration_height(row.output_height, deposit_height);
                        deposited_balance deposited(address_str, tx_hash, deposit_height, expiration_height);
                        if (output_hash == tx_hash) {
                            deposited.balance = row.value;
                        }
                        else {
                            deposited.bonus = row.value;
                            deposited.bonus_hash = output_hash;
                        }
                        sh_vec->push_back(std::move(deposited));
                    }
                }
            }
        }
    }
}

void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance)
{
    auto&& rows = blockchain.get_address_history(address, false);

    uint64_t total_received = 0;
    uint64_t confirmed_balance = 0;
    uint64_t unspent_balance = 0;
    uint64_t frozen_balance = 0;

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows) {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
            && blockchain.get_transaction(tx_temp, tx_height, row.output.hash)) {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            auto output = tx_temp.outputs.at(row.output.index);
            if (output.get_script_address() != address.encoded()) {
                continue;
            }

            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                // deposit utxo in block
                uint64_t lock_height = chain::operation::
                    get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                if (lock_height > blockchain.calc_number_of_blocks(row.output_height, height)) {
                    // utxo already in block but deposit not expire
                    frozen_balance += row.value;
                }
            }
            else if (chain::operation::is_pay_key_hash_with_sequence_lock_pattern(output.script.operations)) {
                uint64_t lock_sequence = chain::operation::
                    get_lock_sequence_from_pay_key_hash_with_sequence_lock(output.script.operations);
                // use any kind of blocks
                if (row.output_height + lock_sequence > height) {
                    // utxo already in block but is locked with sequence and not mature
                    frozen_balance += row.value;
                }
            }
            else if (tx_temp.is_coinbase()) { // coin base etp maturity etp check
                // add not coinbase_maturity etp into frozen
                if (coinbase_maturity > blockchain.calc_number_of_blocks(row.output_height, height)) {
                    frozen_balance += row.value;
                }
            }

            unspent_balance += row.value;
        }

        total_received += row.value;

        if ((row.spend.hash == null_hash || row.spend_height == 0)) {
            confirmed_balance += row.value;
        }
    }

    addr_balance.confirmed_balance = confirmed_balance;
    addr_balance.total_received = total_received;
    addr_balance.unspent_balance = unspent_balance;
    addr_balance.frozen_balance = frozen_balance;
}

void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<utxo_balance::list> sh_vec)
{
    auto&& rows = blockchain.get_address_history(address, false);

    chain::transaction tx_temp;
    uint64_t tx_height = 0;

    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (const auto& row: rows) {
        uint64_t unspent_balance = 0;
        uint64_t frozen_balance = 0;

        if (row.value == 0) {
            continue;
        }

        bool tx_ready = (row.spend.hash == null_hash)
            && blockchain.get_transaction(tx_temp, tx_height, row.output.hash);

        // spend unconfirmed (or no spend attempted)
        if (!tx_ready) {
            continue;
        }

        BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
        auto output = tx_temp.outputs.at(row.output.index);
        if (output.get_script_address() != address.encoded()) {
            continue;
        }

        if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
            // deposit utxo in block
            uint64_t lock_height = chain::operation::
                get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
            if (lock_height > blockchain.calc_number_of_blocks(row.output_height, height)) {
                // utxo already in block but deposit not expire
                frozen_balance += row.value;
            }
        }
        else if (chain::operation::is_pay_key_hash_with_sequence_lock_pattern(output.script.operations)) {
            uint64_t lock_sequence = chain::operation::
                get_lock_sequence_from_pay_key_hash_with_sequence_lock(output.script.operations);
            // use any kind of blocks
            if (row.output_height + lock_sequence > height) {
                // utxo already in block but is locked with sequence and not mature
                frozen_balance += row.value;
            }
        }
        else if (tx_temp.is_coinbase()) { // coin base etp maturity etp check
            // add not coinbase_maturity etp into frozen
            if (coinbase_maturity > blockchain.calc_number_of_blocks(row.output_height, height)) {
                frozen_balance += row.value;
            }
        }

        unspent_balance += row.value;
        sh_vec->emplace_back(utxo_balance{
            encode_hash(row.output.hash), row.output.index,
            row.output_height, unspent_balance, frozen_balance});
    }

    if (sh_vec->size() > 1) {
        auto sort_by_amount_descend = [](const utxo_balance& b1, const utxo_balance& b2){
            return b1.unspent_balance > b2.unspent_balance;
        };
        std::sort(sh_vec->begin(), sh_vec->end(), sort_by_amount_descend);
    }
}

bool base_transfer_common::get_spendable_output(
    chain::output& output, const chain::history& row, uint64_t height) const
{
    // spended
    if (row.spend.hash != null_hash) {
        return false;
    }

    chain::transaction tx_temp;
    uint64_t tx_height;
    if (!blockchain_.get_transaction_consider_pool(tx_temp, tx_height, row.output.hash)) {
        return false;
    }

    const auto is_in_pool = tx_height == max_uint64;

    BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
    output = tx_temp.outputs.at(row.output.index);

    if (exclude_etp_range_.first < exclude_etp_range_.second) {
        if (output.value >= exclude_etp_range_.first && output.value < exclude_etp_range_.second) {
            return false;
        }
    }

    if (is_in_pool) {
        if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
            // deposit utxo in transaction pool
            return false;
        }
        if (chain::operation::is_pay_key_hash_with_sequence_lock_pattern(output.script.operations)) {
            // lock sequence utxo in transaction pool
            return false;
        }
    }

    return blockchain_.is_utxo_spendable(tx_temp, row.output.index, row.output_height, height);
}

// only consider etp and asset and cert.
// specify parameter 'did' to true to only consider did
void base_transfer_common::sync_fetchutxo(
        const std::string& prikey, const std::string& addr, filter filter, const history::list& spec_rows)
{
    auto&& waddr = wallet::payment_address(addr);

    uint64_t height = 0;
    blockchain_.get_last_height(height);

    const auto &rows = spec_rows.empty() ? blockchain_.get_address_history(waddr, true) : spec_rows;

    for (auto& row: rows)
    {
        chain::output output;
        if (!spec_rows.empty()) {
            if (!get_spendable_output(output, row, height)) {
                throw std::logic_error("output spent error.");
            }
        } else {
            // performance improve
            if (is_payment_satisfied(filter)) {
                break;
            }

            if (!get_spendable_output(output, row, height)) {
                continue;
            }

            if (output.get_script_address() != addr) {
                continue;
            }
        }

        auto etp_amount = row.value;
        auto asset_total_amount = output.get_asset_amount();
        auto cert_type = output.get_asset_cert_type();
        auto asset_symbol = output.get_asset_symbol();

        // filter output
        if ((filter & FILTER_ETP) && output.is_etp()) { // etp related
            BITCOIN_ASSERT(asset_total_amount == 0);
            BITCOIN_ASSERT(asset_symbol.empty());
            if (etp_amount == 0)
                continue;
            // enough etp to pay
            if (unspent_etp_ >= payment_etp_)
                continue;
        }
        else if ((filter & FILTER_ASSET) && output.is_asset()) { // asset related
            BITCOIN_ASSERT(etp_amount == 0);
            BITCOIN_ASSERT(cert_type == asset_cert_ns::none);
            if (asset_total_amount == 0)
                continue;
            // enough asset to pay
            if (unspent_asset_ >= payment_asset_)
                continue;
            // check asset symbol
            if (symbol_ != asset_symbol)
                continue;

            if (bc::wallet::symbol::is_forbidden(asset_symbol)) {
                // swallow forbidden symbol
                continue;
            }
        }
        else if ((filter & FILTER_IDENTIFIABLE_ASSET) && output.is_asset_mit()) {
            BITCOIN_ASSERT(etp_amount == 0);
            BITCOIN_ASSERT(asset_total_amount == 0);
            BITCOIN_ASSERT(cert_type == asset_cert_ns::none);

            if (payment_mit_ <= unspent_mit_) {
                continue;
            }

            if (symbol_ != output.get_asset_symbol())
                continue;

            ++unspent_mit_;
        }
        else if ((filter & FILTER_ASSETCERT) && output.is_asset_cert()) { // cert related
            BITCOIN_ASSERT(etp_amount == 0);
            BITCOIN_ASSERT(asset_total_amount == 0);
            // no needed asset cert is included in this output
            if (payment_asset_cert_.empty())
                continue;

            // check cert symbol
            if (cert_type == asset_cert_ns::domain) {
                auto&& domain = asset_cert::get_domain(symbol_);
                if (domain != asset_symbol)
                    continue;
            }
            else {
                if (symbol_ != asset_symbol)
                    continue;
            }

            // check cert type
            if (!asset_cert::test_certs(payment_asset_cert_, cert_type)) {
                continue;
            }

            // asset cert has already found
            if (asset_cert::test_certs(unspent_asset_cert_, payment_asset_cert_)) {
                continue;
            }
        }
        else if ((filter & FILTER_DID) &&
                 (output.is_did_register() || output.is_did_transfer())) { // did related
            BITCOIN_ASSERT(etp_amount == 0);
            BITCOIN_ASSERT(asset_total_amount == 0);
            BITCOIN_ASSERT(cert_type == asset_cert_ns::none);

            if (payment_did_ <= unspent_did_) {
                continue;
            }

            if (symbol_ != output.get_did_symbol())
                continue;

            ++unspent_did_;
        }
        else {
            continue;
        }

        auto asset_amount = asset_total_amount;
        std::shared_ptr<data_chunk> new_model_param_ptr;
        if (asset_total_amount
            && operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
            const auto& attenuation_model_param = output.get_attenuation_model_param();
            new_model_param_ptr = std::make_shared<data_chunk>();
            auto diff_height = row.output_height
                ? blockchain_.calc_number_of_blocks(row.output_height, height)
                : 0;
            asset_amount = attenuation_model::get_available_asset_amount(
                    asset_total_amount, diff_height, attenuation_model_param, new_model_param_ptr);
            if ((asset_amount == 0) && !is_locked_asset_as_payment()) {
                continue; // all locked, filter out
            }
        }

        BITCOIN_ASSERT(asset_total_amount >= asset_amount);

        auto lock_sequence = output.get_lock_sequence();

        if (locktime_ > 0) {
            // ref. is_locktime_conflict() [BIP65]
            if (lock_sequence != max_input_sequence) {
                continue;
            }
            lock_sequence = relative_locktime_disabled;
        }

        // add to from list
        address_asset_record record;

        if (!prikey.empty()) { // raw tx has no prikey
            record.prikey = prikey;
            record.script = output.script;
        }
        record.addr = output.get_script_address();
        record.amount = etp_amount;
        record.symbol = asset_symbol;
        record.asset_amount = asset_amount;
        record.asset_cert = cert_type;
        record.output = row.output;
        record.type = get_utxo_attach_type(output);
        record.sequence = lock_sequence;

        from_list_.push_back(record);

        unspent_etp_ += record.amount;
        unspent_asset_ += record.asset_amount;

        if (record.asset_cert != asset_cert_ns::none) {
            unspent_asset_cert_.push_back(record.asset_cert);
        }

        // asset_locked_transfer as a special change
        if (new_model_param_ptr && (asset_total_amount > record.asset_amount)) {
            auto locked_asset = asset_total_amount - record.asset_amount;
            std::string model_param(new_model_param_ptr->begin(), new_model_param_ptr->end());
            receiver_list_.push_back({record.addr, record.symbol,
                                      0, locked_asset, utxo_attach_type::asset_locked_transfer,
                                      attachment(0, 0, chain::blockchain_message(std::move(model_param))), record.output});
            // in secondary issue, locked asset can also verify threshold condition
            if (is_locked_asset_as_payment()) {
                payment_asset_ = (payment_asset_ > locked_asset)
                                 ? (payment_asset_ - locked_asset) : 0;
            }
        }
    }
}

void base_transfer_common::check_fee_in_valid_range(uint64_t fee)
{
    if ((fee < minimum_fee) || (fee > maximum_fee)) {
        throw asset_exchange_poundage_exception{"fee must in ["
            + std::to_string(minimum_fee) + ", " + std::to_string(maximum_fee) + "]"};
    }
}

void base_transfer_common::check_model_param_initial(std::string& param, uint64_t amount)
{
    if (!param.empty()) {
        if (!validate_transaction::is_nova_feature_activated(blockchain_)) {
            throw asset_attenuation_model_exception(
                "attenuation model should be supported after nova feature is activated.");
        }
        if (!attenuation_model::check_model_param_initial(param, amount, true)) {
            throw asset_attenuation_model_exception("check asset attenuation model param failed");
        }
    }
}

void base_transfer_common::sum_payments()
{
    for (auto& iter : receiver_list_) {
        payment_etp_ += iter.amount;
        payment_asset_ += iter.asset_amount;

        if (iter.asset_cert != asset_cert_ns::none
            && iter.type != utxo_attach_type::asset_cert_autoissue) {
            payment_asset_cert_.push_back(iter.asset_cert);
        }

        if (iter.type == utxo_attach_type::asset_mit_transfer) {
            ++payment_mit_;
            if (payment_mit_ > 1) {
                throw std::logic_error{"maximum one MIT can be transfered"};
            }
        }
        else if (iter.type == utxo_attach_type::did_transfer) {
            ++payment_did_;
            if (payment_did_ > 1) {
                throw std::logic_error{"maximum one DID can be transfered"};
            }
        }
    }
}

void base_transfer_common::check_receiver_list_not_empty() const
{
    if (receiver_list_.empty()) {
        throw toaddress_empty_exception{"empty target address"};
    }
}

void base_transfer_common::sum_payment_amount()
{
    check_receiver_list_not_empty();
    check_fee_in_valid_range(payment_etp_);
    sum_payments();
}

bool base_transfer_common::is_payment_satisfied(filter filter) const
{
    if ((filter & FILTER_ETP) && (unspent_etp_ < payment_etp_))
        return false;

    if ((filter & FILTER_ASSET) && (unspent_asset_ < payment_asset_))
        return false;

    if ((filter & FILTER_IDENTIFIABLE_ASSET) && (unspent_mit_ < payment_mit_))
        return false;

    if ((filter & FILTER_ASSETCERT)
        && !asset_cert::test_certs(unspent_asset_cert_, payment_asset_cert_))
        return false;

    if ((filter & FILTER_DID) && (unspent_did_ < payment_did_))
        return false;

    return true;
}

void base_transfer_common::check_payment_satisfied(filter filter) const
{
    if ((filter & FILTER_ETP) && (unspent_etp_ < payment_etp_)) {
        throw account_balance_lack_exception{"not enough balance, unspent = "
            + std::to_string(unspent_etp_) + ", payment = " + std::to_string(payment_etp_)};
    }

    if ((filter & FILTER_ASSET) && (unspent_asset_ < payment_asset_)) {
        throw asset_lack_exception{"not enough asset amount, unspent = "
            + std::to_string(unspent_asset_) + ", payment = " + std::to_string(payment_asset_)};
    }

    if ((filter & FILTER_IDENTIFIABLE_ASSET) && (unspent_mit_ < payment_mit_)) {
        throw asset_lack_exception{"not enough MIT amount, unspent = "
            + std::to_string(unspent_mit_) + ", payment = " + std::to_string(payment_mit_)};
    }

    if ((filter & FILTER_ASSETCERT)
        && !asset_cert::test_certs(unspent_asset_cert_, payment_asset_cert_)) {
        std::string payment(" ");
        for (auto& cert_type : payment_asset_cert_) {
            payment += asset_cert::get_type_name(cert_type);
            payment += " ";
        }
        std::string unspent(" ");
        for (auto& cert_type : unspent_asset_cert_) {
            unspent += asset_cert::get_type_name(cert_type);
            unspent += " ";
        }

        throw asset_cert_exception{"not enough asset cert, unspent = ("
            + unspent + "), payment = (" + payment + ")"};
    }

    if ((filter & FILTER_DID) && (unspent_did_ < payment_did_)) {
        throw tx_source_exception{"no did named " + symbol_ + " is found"};
    }
}

void base_transfer_common::populate_change()
{
    // only etp utxo, others in derived class
    populate_etp_change();
}

std::string base_transfer_common::get_mychange_address(filter filter) const
{
    if (!mychange_.empty()) {
        return mychange_;
    }

    if ((filter & FILTER_PAYFROM) && !from_.empty()) {
        return from_;
    }

    const auto match = [filter](const address_asset_record& record) {
        if (filter & FILTER_ETP) {
            return (record.type == utxo_attach_type::etp);
        }
        if (filter & FILTER_ASSET) {
            return (record.type == utxo_attach_type::asset_transfer)
                || (record.type == utxo_attach_type::asset_issue)
                || (record.type == utxo_attach_type::asset_secondaryissue);
        }
        throw std::logic_error{"get_mychange_address: unknown/wrong filter for mychange"};
    };

    // reverse find the lates matched unspent
    auto it = from_list_.crbegin();
    for (; it != from_list_.crend(); ++it) {
        if (match(*it)) {
            return it->addr;
        }
    }
    BITCOIN_ASSERT(it != from_list_.crend());

    return from_list_.begin()->addr;
}

void base_transfer_common::populate_etp_change(const std::string& address)
{
    // etp utxo
    if (unspent_etp_ > payment_etp_) {
        auto addr = address;
        if (addr.empty()) {
            addr = get_mychange_address(FILTER_ETP);
        }
        BITCOIN_ASSERT(!addr.empty());

        if (blockchain_.is_valid_address(addr)) {
            receiver_list_.push_back(
                {addr, "", unspent_etp_ - payment_etp_, 0, utxo_attach_type::etp, attachment()});
        }
        else {
            if (addr.length() > DID_DETAIL_SYMBOL_FIX_SIZE) {
                throw did_symbol_length_exception{
                    "mychange did symbol " + addr + " length must be less than 64."};
            }

            auto diddetail = blockchain_.get_registered_did(addr);
            if (!diddetail) {
                throw did_symbol_notfound_exception{
                    "mychange did symbol " + addr + "does not exist on the blockchain"};
            }

            attachment attach;
            attach.set_version(DID_ATTACH_VERIFY_VERSION);
            attach.set_to_did(addr);
            receiver_list_.push_back(
                {diddetail->get_address(), "", unspent_etp_ - payment_etp_, 0, utxo_attach_type::etp, attach});
        }
    }
}

void base_transfer_common::populate_asset_change(const std::string& address)
{
    // asset utxo
    if (unspent_asset_ > payment_asset_) {
        auto addr = address;
        if (addr.empty()) {
            addr = get_mychange_address(FILTER_ASSET);
        }
        BITCOIN_ASSERT(!addr.empty());

        if (blockchain_.is_valid_address(addr)) {
            receiver_list_.push_back({addr, symbol_, 0, unspent_asset_ - payment_asset_,
                utxo_attach_type::asset_transfer, attachment()});
        }
        else {
            if (addr.length() > DID_DETAIL_SYMBOL_FIX_SIZE) {
                throw did_symbol_length_exception{
                    "mychange did symbol " + addr + " length must be less than 64."};
            }

            auto diddetail = blockchain_.get_registered_did(addr);
            if (!diddetail) {
                throw did_symbol_notfound_exception{
                    "mychange did symbol " + addr + "does not exist on the blockchain"};
            }

            attachment attach;
            attach.set_version(DID_ATTACH_VERIFY_VERSION);
            attach.set_to_did(addr);
            receiver_list_.push_back({diddetail->get_address(), symbol_, 0, unspent_asset_ - payment_asset_,
                utxo_attach_type::asset_transfer, attach});
        }
    }
}

chain::operation::stack
base_transfer_common::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and asset should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    const auto& hash = payment.hash();
    if (blockchain_.is_blackhole_address(record.target)) {
        payment_ops = chain::operation::to_pay_blackhole_pattern(hash);
    }
    else if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        if (record.type == utxo_attach_type::asset_locked_transfer) { // for asset locked change only
            const auto& attenuation_model_param =
                boost::get<blockchain_message>(record.attach_elem.get_attach()).get_content();
            if (!attenuation_model::check_model_param_format(to_chunk(attenuation_model_param))) {
                throw asset_attenuation_model_exception(
                    "check asset locked transfer attenuation model param failed: "
                    + attenuation_model_param);
            }
            payment_ops = chain::operation::to_pay_key_hash_with_attenuation_model_pattern(
                hash, attenuation_model_param, record.input_point);
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash);
        }
    }
    else if (payment.version() == wallet::payment_address::mainnet_p2sh) {
        payment_ops = chain::operation::to_pay_script_hash_pattern(hash);
    }
    else {
        throw toaddress_unrecognized_exception{"unrecognized target address : " + payment.encoded()};
    }

    return payment_ops;
}

chain::operation::stack
base_transfer_common::get_pay_key_hash_with_attenuation_model_operations(
    const std::string& model_param, const receiver_record& record)
{
    if (model_param.empty()) {
        throw asset_attenuation_model_exception("attenuation model param is empty.");
    }

    const wallet::payment_address payment(record.target);
    if (!payment) {
        throw toaddress_invalid_exception{"invalid target address"};
    }

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        return chain::operation::to_pay_key_hash_with_attenuation_model_pattern(
                payment.hash(), model_param, record.input_point);
    }

    throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
}

void base_transfer_common::populate_tx_outputs()
{
    for (const auto& iter: receiver_list_) {
        if (iter.is_empty()) {
            continue;
        }

        if (tx_item_idx_ >= (tx_limit + 10)) {
            throw tx_validate_exception{"Too many inputs/outputs,tx too large, canceled."};
        }
        tx_item_idx_++;

        auto&& payment_script = chain::script{ get_script_operations(iter) };

        // generate asset info
        auto&& output_att = populate_output_attachment(iter);
        set_did_verify_attachment(iter, output_att);

        if (!output_att.is_valid()) {
            throw tx_validate_exception{"validate transaction failure, invalid output attachment."};
        }

        // fill output
        tx_.outputs.push_back({ iter.amount, payment_script, output_att });
    }
}

void base_transfer_common::populate_tx_inputs()
{
    // input args
    tx_input_type input;

    for (auto& fromeach : from_list_){
        if (tx_item_idx_ >= tx_limit) {
            auto&& response = "Too many inputs, suggest less than "
                + std::to_string(tx_limit) + " inputs.";
            throw tx_validate_exception(response);
        }
        input.sequence = fromeach.sequence;
        input.previous_output.hash = fromeach.output.hash;
        input.previous_output.index = fromeach.output.index;
        tx_.inputs.push_back(input);
        tx_item_idx_++;
    }
}

void base_transfer_common::set_did_verify_attachment(const receiver_record& record, attachment& attach)
{
    if (record.attach_elem.get_version() == DID_ATTACH_VERIFY_VERSION) {
        attach.set_version(DID_ATTACH_VERIFY_VERSION);
        attach.set_to_did(record.attach_elem.get_to_did());
        attach.set_from_did(record.attach_elem.get_from_did());
    }
}

attachment base_transfer_common::populate_output_attachment(const receiver_record& record)
{
    if ((record.type == utxo_attach_type::etp)
        || (record.type == utxo_attach_type::deposit)
        || ((record.type == utxo_attach_type::asset_transfer)
            && ((record.amount > 0) && (!record.asset_amount)))) { // etp
        return attachment(ETP_TYPE, attach_version, chain::etp(record.amount));
    }
    else if (record.type == utxo_attach_type::asset_issue
        || record.type == utxo_attach_type::asset_secondaryissue) {
        return attachment(ASSET_TYPE, attach_version, asset(/*set on subclass*/));
    }
    else if (record.type == utxo_attach_type::asset_transfer
            || record.type == utxo_attach_type::asset_locked_transfer
            || record.type == utxo_attach_type::asset_attenuation_transfer) {
        auto transfer = chain::asset_transfer(record.symbol, record.asset_amount);
        auto ass = asset(ASSET_TRANSFERABLE_TYPE, transfer);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid asset transfer attachment"};
        }
        return attachment(ASSET_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::message) {
        auto msg = boost::get<blockchain_message>(record.attach_elem.get_attach());
        if (msg.get_content().size() >= 0xfd/* 253, see serializer.ipp */) {
            throw tx_attachment_value_exception{"memo text length should be less than 253"};
        }
        if (!msg.is_valid()) {
            throw tx_attachment_value_exception{"invalid message attachment"};
        }
        return attachment(MESSAGE_TYPE, attach_version, msg);
    }
    else if (record.type == utxo_attach_type::did_register) {
        did_detail diddetail(symbol_, record.target);
        auto ass = did(DID_DETAIL_TYPE, diddetail);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid did register attachment"};
        }
        return attachment(DID_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::did_transfer) {
        auto sh_did = blockchain_.get_registered_did(symbol_);
        if(!sh_did)
            throw did_symbol_notfound_exception{symbol_ + " not found"};

        sh_did->set_address(record.target);
        auto ass = did(DID_TRANSFERABLE_TYPE, *sh_did);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid did transfer attachment"};
        }
        return attachment(DID_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::asset_cert
        || record.type == utxo_attach_type::asset_cert_autoissue
        || record.type == utxo_attach_type::asset_cert_issue
        || record.type == utxo_attach_type::asset_cert_transfer) {
        if (record.asset_cert == asset_cert_ns::none) {
            throw asset_cert_exception("asset cert is none");
        }

        auto to_did = record.attach_elem.get_to_did();
        auto to_address = get_address_from_did(to_did, blockchain_);
        if (to_address != record.target) {
            throw asset_cert_exception("address " + to_address + " dismatch did " + to_did);
        }

        auto cert_info = chain::asset_cert(record.symbol, to_did, to_address, record.asset_cert);
        if (record.type == utxo_attach_type::asset_cert_issue) {
            cert_info.set_status(ASSET_CERT_ISSUE_TYPE);
        }
        else if (record.type == utxo_attach_type::asset_cert_transfer) {
            cert_info.set_status(ASSET_CERT_TRANSFER_TYPE);
        }
        else if (record.type == utxo_attach_type::asset_cert_autoissue) {
            cert_info.set_status(ASSET_CERT_AUTOISSUE_TYPE);
        }

        if (!cert_info.is_valid()) {
            throw tx_attachment_value_exception{"invalid cert attachment"};
        }
        return attachment(ASSET_CERT_TYPE, attach_version, cert_info);
    }
    else if (record.type == utxo_attach_type::asset_mit
        || record.type == utxo_attach_type::asset_mit_transfer) {
        return attachment(ASSET_MIT_TYPE, attach_version, asset_mit(/*set on subclass*/));
    }

    throw tx_attachment_value_exception{
        "invalid utxo_attach_type value in receiver_record : "
            + std::to_string((uint32_t)record.type)};
}

bool base_transfer_common::filter_out_address(const std::string& address) const
{
    return blockchain_.is_script_address(address);
}

void base_transfer_helper::populate_unspent_list()
{
    // get address list
    auto pvaddr = blockchain_.get_account_addresses(name_);
    if (!pvaddr) {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    // get from address balances
    for (auto& each : *pvaddr) {
        const auto& address = each.get_address();
        // filter script address
        if (filter_out_address(address)) {
            continue;
        }

        const auto priv_key = each.get_prv_key(passwd_);

        if (from_.empty()) {
            sync_fetchutxo(priv_key, address);
        } else if (from_ == address) {
            sync_fetchutxo(priv_key, address);
            // select etp/asset utxo only in from_ address
            check_payment_satisfied(FILTER_PAYFROM);
        } else {
            sync_fetchutxo(priv_key, address, FILTER_ALL_BUT_PAYFROM);
        }

        // performance improve
        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough etp or asset in from address"
            ", or you do not own the from address!"};
    }

    check_payment_satisfied();

    populate_change();
}

bool receiver_record::is_empty() const
{
    // has etp amount
    if (amount != 0) {
        return false;
    }

    // etp business , etp == 0
    if ((type == utxo_attach_type::etp) ||
        (type == utxo_attach_type::deposit)) {
        return true;
    }

    // has asset amount
    if (asset_amount != 0) {
        return false;
    }

    // asset transfer business, etp == 0 && asset_amount == 0
    if ((type == utxo_attach_type::asset_transfer) ||
        (type == utxo_attach_type::asset_locked_transfer)) {
        return true;
    }

    // other business
    return false;
}

void base_transfer_common::check_tx()
{
    if (tx_.is_locktime_conflict()) {
        throw tx_locktime_exception{"The specified lock time is ineffective because all sequences"
            " are set to the maximum value."};
    }

    if (tx_.inputs.empty()) {
        throw tx_validate_exception{"validate transaction failure, empty inputs."};
    }

    if (tx_.outputs.empty()) {
        throw tx_validate_exception{"validate transaction failure, empty outputs."};
    }
}

std::string base_transfer_common::get_sign_tx_multisig_script(const address_asset_record& from) const
{
    return "";
}

void base_transfer_common::sign_tx_inputs()
{
    uint32_t index = 0;
    for (auto& fromeach : from_list_)
    {
        bc::chain::script ss;

        // paramaters
        explorer::config::hashtype sign_type;
        uint8_t hash_type = (signature_hash_algorithm)sign_type;

        bc::explorer::config::ec_private config_private_key(fromeach.prikey);
        const ec_secret& private_key = config_private_key;

        std::string multisig_script = get_sign_tx_multisig_script(fromeach);
        if (!multisig_script.empty()) {
            bc::explorer::config::script config_contract(multisig_script);
            const bc::chain::script &contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                                                       contract, tx_, index, hash_type))
            {
                throw tx_sign_exception{"get_input_sign sign failure"};
            }

            // do script
            ss.operations.push_back({bc::chain::opcode::zero, {}});
            ss.operations.push_back({bc::chain::opcode::special, endorse});

            chain::script script_encoded;
            script_encoded.from_string(multisig_script);

            ss.operations.push_back(chain::operation::from_raw_data(script_encoded.to_data(false)));
        }
        else {
            bc::explorer::config::script config_contract(fromeach.script);
            const bc::chain::script& contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                contract, tx_, index, hash_type))
            {
                throw tx_sign_exception{"get_input_sign sign failure"};
            }

            // do script
            bc::wallet::ec_private ec_private_key(private_key, 0u, true);
            auto&& public_key = ec_private_key.to_public();
            data_chunk public_key_data;
            public_key.to_data(public_key_data);

            ss.operations.push_back({bc::chain::opcode::special, endorse});
            ss.operations.push_back({bc::chain::opcode::special, public_key_data});

            // if pre-output script is deposit tx.
            if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height) {
                uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                    contract.operations);
                ss.operations.push_back({bc::chain::opcode::special, script_number(lock_height).data()});
            }
        }

        // set input script of this tx
        tx_.inputs[index].script = ss;
        index++;
    }
}

void base_transfer_common::send_tx()
{
    if(blockchain_.validate_transaction(tx_)) {
#ifdef MVS_DEBUG
        throw tx_validate_exception{"validate transaction failure. " + tx_.to_string(1)};
#endif
        throw tx_validate_exception{"validate transaction failure"};
    }
    if(blockchain_.broadcast_transaction(tx_))
        throw tx_broadcast_exception{"broadcast transaction failure"};
}

void base_transfer_common::populate_tx_header()
{
#ifdef ENABLE_LOCKTIME
    tx_.locktime = locktime_;
#endif

    if (validate_transaction::is_nova_feature_activated(blockchain_)) {
        tx_.version = transaction_version::check_nova_feature;
    }
    else {
        tx_.version = transaction_version::check_output_script;
    }
}

void base_transfer_common::exec()
{
    // prepare
    sum_payment_amount();
    populate_unspent_list();

    // construct tx
    populate_tx_header();
    populate_tx_inputs();
    populate_tx_outputs();

    // check tx
    check_tx();

    // sign tx
    sign_tx_inputs();

    // send tx
    send_tx();
}

void base_multisig_transfer_helper::send_tx()
{
    auto from_address = multisig_.get_address();
    if (from_address.empty()) {
        base_transfer_common::send_tx();
    }
    else {
        // no operation in exec for transferring multisig asset cert
    }
}

bool base_multisig_transfer_helper::filter_out_address(const std::string& address) const
{
    auto multisig_address = multisig_.get_address();
    if (multisig_address.empty()) {
        return base_transfer_common::filter_out_address(address);
    }
    else {
        return address != multisig_address;
    }
}

std::string base_multisig_transfer_helper::get_sign_tx_multisig_script(const address_asset_record& from) const
{
    return multisig_.get_multisig_script();
}

void base_transaction_constructor::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (from_vec_.empty() && from_list_.empty()) {
        throw fromaddress_empty_exception{"empty from address"};
    }
}

void base_transaction_constructor::populate_change()
{
    // etp utxo
    populate_etp_change();

    // asset utxo
    populate_asset_change();

    if (!message_.empty()) { // etp transfer/asset transfer  -- with message
        auto addr = !mychange_.empty() ? mychange_ : from_list_.begin()->addr;
        receiver_list_.push_back({addr, "", 0, 0,
            utxo_attach_type::message,
            attachment(0, 0, chain::blockchain_message(message_))});
    }
}

void base_transaction_constructor::populate_unspent_list()
{
    // get from address balances
    for (auto& each : from_vec_) {
        sync_fetchutxo("", each);
        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough etp or asset in the from address!"};
    }

    check_payment_satisfied();

    // change
    populate_change();
}

const std::vector<uint16_t> depositing_etp::vec_cycle{7, 30, 90, 182, 365};

uint32_t depositing_etp::get_reward_lock_height() const
{
    int index = 0;
    auto it = std::find(vec_cycle.begin(), vec_cycle.end(), deposit_cycle_);
    if (it != vec_cycle.end()) { // found cycle
        index = std::distance(vec_cycle.begin(), it);
    }

    return (uint32_t)bc::consensus::lock_heights[index];
}

chain::operation::stack
depositing_etp::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and asset should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        const auto& hash = payment.hash();
        if((to_ == record.target)
            && (utxo_attach_type::deposit == record.type)) {
            payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(hash, get_reward_lock_height());
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
        }
    }
    else {
        throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
    }

    return payment_ops;
}

const std::vector<uint16_t> depositing_etp_transaction::vec_cycle{7, 30, 90, 182, 365};

uint32_t depositing_etp_transaction::get_reward_lock_height() const
{
    int index = 0;
    auto it = std::find(vec_cycle.begin(), vec_cycle.end(), deposit_);
    if (it != vec_cycle.end()) { // found cycle
        index = std::distance(vec_cycle.begin(), it);
    }

    return (uint32_t)bc::consensus::lock_heights[index];
}

chain::operation::stack
depositing_etp_transaction::get_script_operations(const receiver_record& record) const
{
    chain::operation::stack payment_ops;

    // complicated script and asset should be implemented in subclass
    // generate script
    const wallet::payment_address payment(record.target);
    if (!payment)
        throw toaddress_invalid_exception{"invalid target address"};

    if (payment.version() == wallet::payment_address::mainnet_p2kh) {
        const auto& hash = payment.hash();
        if((utxo_attach_type::deposit == record.type)) {
            payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(
                hash, get_reward_lock_height());
        } else {
            payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
        }
    }
    else {
        throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
    }

    return payment_ops;
}

void sending_multisig_tx::populate_change()
{
    // etp utxo
    populate_etp_change();

    // asset utxo
    populate_asset_change();
}

void issuing_asset::sum_payments()
{
    for (auto& iter : receiver_list_) {
        payment_etp_ += iter.amount;
        payment_asset_ += iter.asset_amount;

        if (iter.type == utxo_attach_type::asset_cert) {
            auto&& domain = asset_cert::get_domain(symbol_);
            if (!asset_cert::is_valid_domain(domain)) {
                throw asset_cert_domain_exception{"invalid domain for asset : " + symbol_};
            }

            if (iter.asset_cert == asset_cert_ns::domain
                || iter.asset_cert == asset_cert_ns::naming) {
                payment_asset_cert_.push_back(iter.asset_cert); // will verify by input
            }
        }
    }
}

void issuing_asset::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    unissued_asset_ = blockchain_.get_account_unissued_asset(name_, symbol_);
    if (!unissued_asset_) {
        throw asset_symbol_notfound_exception{symbol_ + " not created"};
    }

    uint64_t min_fee = bc::min_fee_to_issue_asset;
    if (payment_etp_ < min_fee) {
        throw asset_issue_poundage_exception("fee must at least "
            + std::to_string(min_fee) + " satoshi == "
            + std::to_string(min_fee/100000000) + " etp");
    }

    if (!attenuation_model_param_.empty()) {
        check_model_param_initial(attenuation_model_param_, unissued_asset_->get_maximum_supply());
    }

    uint64_t amount = (uint64_t)std::floor(payment_etp_ * ((100 - fee_percentage_to_miner_) / 100.0));
    if (amount > 0) {
        auto&& address = bc::get_developer_community_address(blockchain_.chain_settings().use_testnet_rules);
        auto&& did = blockchain_.get_did_from_address(address);
        receiver_list_.push_back({address, "", amount, 0, utxo_attach_type::etp, attachment("", did)});
    }
}

chain::operation::stack
issuing_asset::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::asset_issue == record.type)) {
        return get_pay_key_hash_with_attenuation_model_operations(attenuation_model_param_, record);
    }

    return base_transfer_helper::get_script_operations(record);
}

attachment issuing_asset::populate_output_attachment(const receiver_record& record)
{
    attachment&& attach = base_transfer_common::populate_output_attachment(record);

    if (record.type == utxo_attach_type::asset_issue) {
        unissued_asset_->set_address(record.target);
        auto ass = asset(ASSET_DETAIL_TYPE, *unissued_asset_);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid asset issue attachment"};
        }

        attach.set_attach(ass);
    }
    else if (record.type == utxo_attach_type::asset_cert_autoissue
        && record.asset_cert == asset_cert_ns::mining) {
        auto& cert_info = boost::get<asset_cert>(attach.get_attach());
        cert_info.set_content(mining_sussidy_param_);
        BITCOIN_ASSERT(cert_info.has_content());
    }

    return attach;
}

void sending_asset::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    if (!attenuation_model_param_.empty()) {
        check_model_param_initial(attenuation_model_param_, payment_asset_);
    }
}

void sending_asset::populate_change()
{
    // etp utxo
    populate_etp_change();

    // asset utxo
    populate_asset_change();

    // asset transfer  -- with message
    if (!message_.empty()) {
        auto addr = !mychange_.empty() ? mychange_ : from_list_.begin()->addr;
        receiver_list_.push_back({addr, "", 0, 0,
            utxo_attach_type::message,
            attachment(0, 0, chain::blockchain_message(message_))});
    }
}

chain::operation::stack
sending_asset::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::asset_attenuation_transfer == record.type)) { // for sending asset only
        return get_pay_key_hash_with_attenuation_model_operations(attenuation_model_param_, record);
    }

    return base_transfer_helper::get_script_operations(record);
}

chain::operation::stack
secondary_issuing_asset::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::asset_secondaryissue == record.type)) {
        return get_pay_key_hash_with_attenuation_model_operations(attenuation_model_param_, record);
    }

    return base_transfer_helper::get_script_operations(record);
}

void secondary_issuing_asset::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    target_address_ = receiver_list_.begin()->target;

    issued_asset_ = blockchain_.get_issued_asset(symbol_);
    if (!issued_asset_) {
        throw asset_symbol_notfound_exception{"asset symbol does not exist on the blockchain"};
    }

    auto total_volume = blockchain_.get_asset_volume(symbol_);
    if (total_volume > max_uint64 - volume_) {
        throw asset_amount_exception{"secondaryissue, volume cannot exceed maximum value"};
    }

    if (!asset_cert::test_certs(payment_asset_cert_, asset_cert_ns::issue)) {
        throw asset_cert_exception("no asset cert of issue right is provided.");
    }

    if (blockchain_.chain_settings().use_testnet_rules
        && !blockchain_.is_asset_cert_exist(symbol_, asset_cert_ns::issue)) {
        payment_asset_cert_.clear();
    }

    if (!attenuation_model_param_.empty()) {
        check_model_param_initial(attenuation_model_param_, volume_);
    }
}

void secondary_issuing_asset::populate_change()
{
    // etp utxo
    populate_etp_change();

    // asset utxo
    if (payment_asset_ > 0) {
        receiver_list_.push_back({target_address_, symbol_,
            0, payment_asset_,
            utxo_attach_type::asset_transfer, attachment()});
    }
    populate_asset_change(target_address_);
}

attachment secondary_issuing_asset::populate_output_attachment(const receiver_record& record)
{
    auto&& attach = base_transfer_common::populate_output_attachment(record);

    if (record.type == utxo_attach_type::asset_secondaryissue) {
        auto asset_detail = *issued_asset_;
        asset_detail.set_address(record.target);
        asset_detail.set_asset_secondaryissue();
        asset_detail.set_maximum_supply(volume_);
        asset_detail.set_issuer(record.attach_elem.get_to_did());
        auto ass = asset(ASSET_DETAIL_TYPE, asset_detail);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid asset secondary issue attachment"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

void issuing_asset_cert::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    if (asset_cert::test_certs(payment_asset_cert_, asset_cert_ns::naming)) {
        if (!asset_cert::test_certs(payment_asset_cert_, asset_cert_ns::domain)) {
            throw asset_cert_exception("no asset cert of domain right.");
        }

        payment_asset_cert_.clear();
        payment_asset_cert_.push_back(asset_cert_ns::domain);
    }
    else {
        payment_asset_cert_.clear();
    }
}

void registering_did::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();

    uint64_t min_fee = bc::min_fee_to_register_did;
    if (payment_etp_ < min_fee) {
        throw did_register_poundage_exception("fee must at least "
            + std::to_string(min_fee) + " satoshi == "
            + std::to_string(min_fee/100000000) + " etp");
    }

    uint64_t amount = (uint64_t)std::floor(payment_etp_ * ((100 - fee_percentage_to_miner_) / 100.0));
    if (amount > 0) {
        auto&& address = bc::get_developer_community_address(blockchain_.chain_settings().use_testnet_rules);
        auto&& did = blockchain_.get_did_from_address(address);
        receiver_list_.push_back({address, "", amount, 0, utxo_attach_type::etp, attachment("", did)});
    }
}

std::string sending_multisig_did::get_sign_tx_multisig_script(const address_asset_record& from) const
{
    std::string multisig_script;
    if (from.addr == multisig_from_.get_address()) {
        multisig_script = multisig_from_.get_multisig_script();

    }
    else if (from.addr == multisig_to_.get_address()) {
        multisig_script = multisig_to_.get_multisig_script();
    }
    return multisig_script;
}

void sending_multisig_did::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (fromfee.empty()) {
        throw fromaddress_empty_exception{"empty fromfee address"};
    }
}

void sending_multisig_did::populate_change()
{
    // etp utxo
    populate_etp_change(fromfee);
}

void sending_multisig_did::populate_unspent_list()
{
    // get address list
    auto pvaddr = blockchain_.get_account_addresses(name_);
    if (!pvaddr) {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    // get from address balances
    for (auto& each : *pvaddr) {

        if (fromfee == each.get_address()) {
            // pay fee
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_ETP);
            check_payment_satisfied(FILTER_ETP);
        }

        if (from_ == each.get_address()) {
            // pay did
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_DID);
            check_payment_satisfied(FILTER_DID);
        }

        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough etp or asset in from address"
            ", or you do not own the from address!"};
    }

    check_payment_satisfied();

    populate_change();
}

void sending_did::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (fromfee.empty()) {
        throw fromaddress_empty_exception{"empty fromfee address"};
    }
}

void sending_did::populate_change()
{
    // etp utxo
    populate_etp_change(fromfee);
}

void sending_did::populate_unspent_list()
{
    // get address list
    auto pvaddr = blockchain_.get_account_addresses(name_);
    if (!pvaddr) {
        throw address_list_nullptr_exception{"nullptr for address list"};
    }

    // get from address balances
    for (auto& each : *pvaddr) {
        // filter script address
        if (blockchain_.is_script_address(each.get_address()))
            continue;

        if (fromfee == each.get_address()) {
            // pay fee
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_ETP);
            check_payment_satisfied(FILTER_ETP);
        }

        if (from_ == each.get_address()) {
            // pay did
            sync_fetchutxo(each.get_prv_key(passwd_), each.get_address(), FILTER_DID);
            check_payment_satisfied(FILTER_DID);
        }

        if (is_payment_satisfied()) {
            break;
        }
    }

    if (from_list_.empty()) {
        throw tx_source_exception{"not enough etp or asset in from address"
            ", or you do not own the from address!"};
    }

    check_payment_satisfied();

    populate_change();
}

attachment registering_mit::populate_output_attachment(const receiver_record& record)
{
    auto&& attach = base_transfer_common::populate_output_attachment(record);

    if (record.type == utxo_attach_type::asset_mit) {
        auto iter = mit_map_.find(record.symbol);
        if (iter == mit_map_.end()) {
            throw tx_attachment_value_exception{"invalid MIT issue attachment"};
        }

        auto ass = asset_mit(record.symbol, record.target, iter->second);
        ass.set_status(MIT_STATUS_REGISTER);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid MIT issue attachment"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

attachment transferring_mit::populate_output_attachment(const receiver_record& record)
{
    auto&& attach = base_transfer_common::populate_output_attachment(record);

    if (record.type == utxo_attach_type::asset_mit_transfer) {
        auto ass = asset_mit(record.symbol, record.target, "");
        ass.set_status(MIT_STATUS_TRANSFER);
        if (!ass.is_valid()) {
            throw tx_attachment_value_exception{"invalid MIT transfer attachment"};
        }

        attach.set_attach(ass);
    }

    return attach;
}

chain::operation::stack lock_sending::get_script_operations(const receiver_record& record) const
{
    if (record.is_lock_seq_) {
        if ((chain::get_script_context() & chain::script_context::bip112_enabled) == 0) {
            throw argument_legality_exception{"lock sequence(bip112) is not enabled"};
        }
        const wallet::payment_address payment(record.target);
        if (!payment) {
            throw toaddress_invalid_exception{"invalid target address"};
        }

        if (payment.version() == wallet::payment_address::mainnet_p2kh) {
            const auto& hash = payment.hash();
            return chain::operation::to_pay_key_hash_with_sequence_lock_pattern(hash, sequence_);
        }
        else {
            throw toaddress_invalid_exception{"not supported target address " + record.target};
        }
    }

    return base_transfer_helper::get_script_operations(record);
}

} //commands
} // explorer
} // libbitcoin
