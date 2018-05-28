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

#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/exception.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {

using bc::chain::blockchain_message;

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
    if (output.is_did_issue()) {
        return utxo_attach_type::did_issue;
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

void check_did_symbol(const std::string& symbol, bool check_sensitive)
{
    if (!chain::output::is_valid_did_symbol(symbol, 0)) {
            throw did_symbol_name_exception{"Did symbol " + symbol + " is not valid."};
    }

    if (check_sensitive)
    {
        // sensitive check
        std::string symbolupper = symbol;
        for (auto &i : symbolupper)
        {
            i = std::toupper(i);
        }

        if (bc::wallet::symbol::is_sensitive(symbolupper))
        {
            throw did_symbol_name_exception{"Did symbol is forbidden."};
        }

        if (symbolupper == "BLACKHOLE")
        {
            throw did_symbol_name_exception{"Did symbol cannot be blackhole."};
        }
    }
}

void check_asset_symbol(const std::string& symbol, bool check_sensitive)
{
    if (symbol.empty()) {
        throw asset_symbol_length_exception{"Symbol cannot be empty."};
    }

    if (symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE) {
        throw asset_symbol_length_exception{"Symbol length must be less than "
            + std::to_string(ASSET_DETAIL_SYMBOL_FIX_SIZE) + "."};
    }

    if (check_sensitive) {
        if (bc::wallet::symbol::is_sensitive(symbol)) {
            throw asset_symbol_name_exception{"Symbol " + symbol + " is forbidden."};
        }
    }
}

std::string get_address_from_did(const std::string& did,
    bc::blockchain::block_chain_impl& blockchain)
{

    check_did_symbol(did);

    auto diddetail = blockchain.get_issued_did(did);
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
    auto&& rows = blockchain.get_address_history(wallet::payment_address(address));

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
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
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height))
        {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);
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
                    auto available_amount = attenuation_model::get_available_asset_amount(
                            asset_amount, height - row.output_height, attenuation_model_param);
                    locked_amount = asset_amount - available_amount;
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

void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance)
{
    auto&& rows = blockchain.get_address_history(address);

    uint64_t total_received = 0;
    uint64_t confirmed_balance = 0;
    uint64_t unspent_balance = 0;
    uint64_t frozen_balance = 0;

    chain::transaction tx_temp;
    uint64_t tx_height;
    uint64_t height = 0;
    blockchain.get_last_height(height);

    for (auto& row: rows)
    {
        total_received += row.value;

        // spend unconfirmed (or no spend attempted)
        if ((row.spend.hash == null_hash)
                && blockchain.get_transaction(row.output.hash, tx_temp, tx_height)) {
            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            auto output = tx_temp.outputs.at(row.output.index);

            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                if (row.output_height == 0) {
                    // deposit utxo in transaction pool
                    frozen_balance += row.value;
                }
                else {
                    // deposit utxo in block
                    uint64_t lock_height = chain::operation::
                        get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                    if((row.output_height + lock_height) > height) {
                        // utxo already in block but deposit not expire
                        frozen_balance += row.value;
                    }
                }
            }
            else if (tx_temp.is_coinbase()) { // coin base etp maturity etp check
                // add not coinbase_maturity etp into frozen
                if ((row.output_height == 0) || ((row.output_height + coinbase_maturity) > height)) {
                    frozen_balance += row.value;
                }
            }

            unspent_balance += row.value;
        }

        if (row.output_height != 0 &&
            (row.spend.hash == null_hash || row.spend_height == 0))
            confirmed_balance += row.value;
    }

    addr_balance.confirmed_balance = confirmed_balance;
    addr_balance.total_received = total_received;
    addr_balance.unspent_balance = unspent_balance;
    addr_balance.frozen_balance = frozen_balance;
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
    if (!blockchain_.get_transaction(row.output.hash, tx_temp, tx_height)) {
        return false;
    }

    BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
    output = tx_temp.outputs.at(row.output.index);

    if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
        if (row.output_height == 0) {
            // deposit utxo in transaction pool
            return false;
        } else {
            // deposit utxo in block
            auto lock_height = chain::operation::
                get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
            if ((row.output_height + lock_height) > height) {
                // utxo already in block but deposit not expire
                return false;
            }
        }
    } else if (tx_temp.is_coinbase()) { // incase readd deposit
        // coin base etp maturity etp check
        // coinbase_maturity etp check
        if ((row.output_height == 0) || ((row.output_height + coinbase_maturity) > height)) {
            return false;
        }
    }

    return true;
}

// only consider etp and asset and cert.
// specify parameter 'did' to true to only consider did
void base_transfer_common::sync_fetchutxo(
        const std::string& prikey, const std::string& addr, filter filter)
{
    auto&& waddr = wallet::payment_address(addr);
    auto&& rows = blockchain_.get_address_history(waddr);

    uint64_t height = 0;
    blockchain_.get_last_height(height);

    for (auto& row: rows)
    {
        // performance improve
        if (is_payment_satisfied(filter)) {
            break;
        }

        chain::output output;
        if (!get_spendable_output(output, row, height)) {
            continue;
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
            (output.is_did_issue() || output.is_did_transfer())) { // did related
            BITCOIN_ASSERT(etp_amount == 0);
            BITCOIN_ASSERT(asset_total_amount == 0);
            BITCOIN_ASSERT(cert_type == asset_cert_ns::none);
            if (symbol_ != output.get_did_symbol())
                continue;
            unspent_did_flag = true;
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
            asset_amount = attenuation_model::get_available_asset_amount(
                    asset_total_amount, height - row.output_height,
                    attenuation_model_param, new_model_param_ptr);
            if ((asset_amount == 0) && !is_locked_asset_as_payment()) {
                continue; // all locked, filter out
            }
        }

        BITCOIN_ASSERT(asset_total_amount >= asset_amount);

        // add to from list
        address_asset_record record;

        if (!prikey.empty()) { // raw tx has no prikey
            record.prikey = prikey;
            record.script = output.script;
        }
        record.addr = addr;
        record.amount = etp_amount;
        record.symbol = asset_symbol;
        record.asset_amount = asset_amount;
        record.asset_cert = cert_type;
        record.output = row.output;
        record.type = get_utxo_attach_type(output);

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
                    attachment(0, 0, blockchain_message(std::move(model_param))), record.output});
            // in secondary issue, locked asset can also verify threshold condition
            if (is_locked_asset_as_payment()) {
                payment_asset_ = (payment_asset_ > locked_asset)
                    ? (payment_asset_ - locked_asset) : 0;
            }
        }
    }

    rows.clear();
}

void base_transfer_common::check_fee_in_valid_range(uint64_t fee)
{
    if ((fee < minimum_fee) || (fee > maximum_fee)) {
        throw asset_exchange_poundage_exception{"fee must in ["
            + std::to_string(minimum_fee) + ", " + std::to_string(maximum_fee) + "]"};
    }
}

void base_transfer_common::sum_payments()
{
    for (auto& iter : receiver_list_) {
        payment_etp_ += iter.amount;
        payment_asset_ += iter.asset_amount;
        if (iter.asset_cert != asset_cert_ns::none) {
            payment_asset_cert_.push_back(iter.asset_cert);
        }
        if (iter.type == utxo_attach_type::did_transfer) {
            if (payment_did_flag) {
                throw std::logic_error{"maximum one DID can be transfered"};
            }
            payment_did_flag = true;
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

    if ((filter & FILTER_ASSETCERT)
        && !asset_cert::test_certs(unspent_asset_cert_, payment_asset_cert_))
        return false;

    if ((filter & FILTER_DID) && (unspent_did_flag != payment_did_flag))
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

    if ((filter & FILTER_DID) && (unspent_did_flag != payment_did_flag)) {
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
                || (record.type == utxo_attach_type::asset_issue);
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

            auto diddetail = blockchain_.get_issued_did(addr);
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

            auto diddetail = blockchain_.get_issued_did(addr);
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
        if (record.type == utxo_attach_type::asset_locked_transfer) {
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

        tx_item_idx_++;
        input.sequence = max_input_sequence;
        input.previous_output.hash = fromeach.output.hash;
        input.previous_output.index = fromeach.output.index;
        tx_.inputs.push_back(input);
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
        attachment attach(ETP_TYPE, attach_version, chain::etp(record.amount));
        return attach;
    }
    else if (record.type == utxo_attach_type::asset_issue
        || record.type == utxo_attach_type::asset_secondaryissue) {
        attachment attach(ASSET_TYPE, attach_version, asset(/*set on subclass*/));
        return attach;
    }
    else if (record.type == utxo_attach_type::asset_transfer
            || record.type == utxo_attach_type::asset_locked_transfer) {
        auto transfer = chain::asset_transfer(record.symbol, record.asset_amount);
        auto ass = asset(ASSET_TRANSFERABLE_TYPE, transfer);
        return attachment(ASSET_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::message) {
        auto msg = boost::get<blockchain_message>(record.attach_elem.get_attach());
        return attachment(MESSAGE_TYPE, attach_version, msg);
    }
    else if (record.type == utxo_attach_type::did_issue) {
        did_detail diddetail(symbol_, record.target);
        auto ass = did(DID_DETAIL_TYPE, diddetail);
        return attachment(DID_TYPE, attach_version, ass);
    }
    else if (record.type == utxo_attach_type::did_transfer) {
        auto sh_did = blockchain_.get_issued_did(symbol_);
        if(!sh_did)
            throw did_symbol_notfound_exception{symbol_ + " not found"};

        sh_did->set_address(record.target);
        auto ass = did(DID_TRANSFERABLE_TYPE, *sh_did);
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

        auto attach = attachment(ASSET_CERT_TYPE, attach_version, cert_info);
        return attach;
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
            data_chunk data;
            ss.operations.push_back({bc::chain::opcode::zero, data});
            ss.operations.push_back({bc::chain::opcode::special, endorse});

            chain::script script_encoded;
            script_encoded.from_string(multisig_script);

            ss.operations.push_back({bc::chain::opcode::pushdata1, script_encoded.to_data(false)});
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
    if (from_vec_.empty()) {
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
            attachment(0, 0, blockchain_message(message_))});
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

        if (iter.asset_cert == asset_cert_ns::domain) {
            auto&& domain = asset_cert::get_domain(symbol_);
            if (!asset_cert::is_valid_domain(domain)) {
                throw asset_cert_domain_exception{"no valid domain exists for asset : " + symbol_};
            }
            if (blockchain_.is_asset_cert_exist(domain, asset_cert_ns::domain)) {
                payment_asset_cert_.clear();
                payment_asset_cert_.push_back(asset_cert_ns::domain); // will verify by input
            }
        }
        else if (iter.asset_cert == asset_cert_ns::naming) {
            auto&& domain = asset_cert::get_domain(symbol_);
            if (!asset_cert::is_valid_domain(domain)) {
                throw asset_cert_domain_exception{"no valid domain exists for asset : " + symbol_};
            }

            if (blockchain_.is_asset_cert_exist(symbol_, asset_cert_ns::naming)) {
                payment_asset_cert_.clear();
                payment_asset_cert_.push_back(asset_cert_ns::naming); // will verify by input
            }
            else {
                throw asset_cert_notfound_exception{"no naming cert exists for asset : " + symbol_};
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

    if (payment_etp_ < 1000000000) { // 10 etp now
        throw asset_issue_poundage_exception{"fee must at least 1000000000 satoshi == 10 etp"};
    }
    if (!attenuation_model_param_.empty()) {
        if (!attenuation_model::check_model_param_initial(
            attenuation_model_param_, unissued_asset_->get_maximum_supply())) {
            throw asset_attenuation_model_exception("check asset attenuation model param failed");
        }
    }
}

chain::operation::stack
issuing_asset::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::asset_issue == record.type)) {

        const wallet::payment_address payment(record.target);
        if (!payment) {
            throw toaddress_invalid_exception{"invalid target address"};
        }

        if (payment.version() == wallet::payment_address::mainnet_p2kh) {
            const auto& hash = payment.hash();
            return chain::operation::to_pay_key_hash_with_attenuation_model_pattern(
                    hash, attenuation_model_param_, record.input_point);
        } else {
            throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
        }
    }

    return base_transfer_helper::get_script_operations(record);
}

attachment issuing_asset::populate_output_attachment(const receiver_record& record)
{
    attachment&& attach = base_transfer_common::populate_output_attachment(record);

    if (record.type == utxo_attach_type::asset_issue) {
        unissued_asset_->set_address(record.target);
        auto ass = asset(ASSET_DETAIL_TYPE, *unissued_asset_);

        attach.set_attach(ass);
    }

    return attach;
}

void sending_asset::populate_change()
{
    // etp utxo
    populate_etp_change();

    // asset utxo
    populate_asset_change();
}

chain::operation::stack
secondary_issuing_asset::get_script_operations(const receiver_record& record) const
{
    if (!attenuation_model_param_.empty()
        && (utxo_attach_type::asset_secondaryissue == record.type)) {

        const wallet::payment_address payment(record.target);
        if (!payment) {
            throw toaddress_invalid_exception{"invalid target address"};
        }

        if (payment.version() == wallet::payment_address::mainnet_p2kh) {
            const auto& hash = payment.hash();
            return chain::operation::to_pay_key_hash_with_attenuation_model_pattern(
                    hash, attenuation_model_param_, record.input_point);
        } else {
            throw toaddress_invalid_exception{std::string("not supported version target address ") + record.target};
        }
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
        if (!attenuation_model::check_model_param_initial(attenuation_model_param_, volume_)) {
            throw asset_attenuation_model_exception("check asset attenuation model param failed");
        }
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
}

void issuing_did::sum_payment_amount()
{
    base_transfer_common::sum_payment_amount();
    if (payment_etp_ < 100000000) {
        throw did_issue_poundage_exception{"fee must at least 100000000 satoshi == 1 etp"};
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

} //commands
} // explorer
} // libbitcoin
