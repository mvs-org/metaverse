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
#pragma once

#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/command.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands{

/// NOTICE: this type is not equal to attachment_type and business_kind
/// attachment_type : the collapsed type of tx output attachment, **recorded on blockchain**
/// business_kind   : the expanded type of attachment, mainly used for database/history query
/// for example :
/// attachment_type           |  business_kind
/// -------------------------------------------------------------------
/// attachment_etp           --> etp
/// attachment_etp_award     --> etp_award
/// attachment_asset         --> asset_issue | asset_transfer
/// attachment_message       --> message
/// attachment_did           --> did_issue   |  did_transfer
/// attachment_asset_cert    --> asset_cert
/// -------------------------------------------------------------------
/// utxo_attach_type is only used in explorer module
/// utxo_attach_type will be used to generate attachment with attachment_type and content
/// for example :
/// utxo_attach_type::asset_issue    --> attachment_asset of asset_detail
///     auto asset_detail = asset(ASSET_DETAIL_TYPE, asset_detail);
///     attachment(ASSET_TYPE, attach_version, asset_detail);
/// utxo_attach_type::asset_transfer --> attachment_asset of asset_transfer
///     auto asset_transfer = asset(ASSET_TRANSFERABLE_TYPE, asset_transfer);
///     attachment(ASSET_TYPE, attach_version, asset_transfer);
enum class utxo_attach_type : uint32_t
{
    etp = 0,
    deposit = 1,
    asset_issue = 2,
    asset_transfer = 3,
    asset_locked_issue = 4,
    asset_locked_transfer = 5,
    message = 6,
    asset_cert = 7,
    asset_secondaryissue = 8,
    did_issue = 9,
    did_transfer = 10,
    invalid = 0xffffffff
};

extern utxo_attach_type get_utxo_attach_type(const chain::output&);

struct address_asset_record{
    std::string prikey;
    std::string addr;
    uint64_t    amount{0}; // spendable etp amount
    std::string symbol;
    uint64_t    asset_amount{0}; // spendable asset amount
    asset_cert_type asset_cert{asset_cert_ns::none};
    utxo_attach_type type{utxo_attach_type::invalid};
    output_point output;
    chain::script script;
    uint32_t hd_index{0}; // only used for multisig tx
};


struct receiver_record {
    std::string target;
    std::string symbol;
    uint64_t    amount; // etp value
    uint64_t    asset_amount;
    asset_cert_type asset_cert;

    utxo_attach_type type;
    attachment attach_elem;  // used for MESSAGE_TYPE, used for information transfer etc.

    receiver_record()
        : target(), symbol()
        , amount(0), asset_amount(0), asset_cert(asset_cert_ns::none)
        , type(utxo_attach_type::invalid), attach_elem()
    {}

    receiver_record(std::string target_, std::string symbol_,
        uint64_t amount_, uint64_t asset_amount_,
        utxo_attach_type type_, attachment attach_elem_)
        : target(target_), symbol(symbol_)
        , amount(amount_), asset_amount(asset_amount_), asset_cert(asset_cert_ns::none)
        , type(type_), attach_elem(attach_elem_)
    {}

    receiver_record(std::string target_, std::string symbol_,
        uint64_t amount_, uint64_t asset_amount_, asset_cert_type asset_cert_,
        utxo_attach_type type_, attachment attach_elem_)
        : target(target_), symbol(symbol_)
        , amount(amount_), asset_amount(asset_amount_), asset_cert(asset_cert_)
        , type(type_), attach_elem(attach_elem_)
    {}

    bool is_empty() const;

};

struct balances {
    uint64_t total_received;
    uint64_t confirmed_balance;
    uint64_t unspent_balance;
    uint64_t frozen_balance;
};

// helper function
void sync_fetchbalance (wallet::payment_address& address, std::string& type,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance, uint64_t amount);
void sync_fetch_asset_balance (std::string& addr,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<std::vector<asset_detail>> sh_asset_vec);
void sync_fetch_asset_balance_record (std::string& addr,
    bc::blockchain::block_chain_impl& blockchain, std::shared_ptr<std::vector<asset_detail>> sh_asset_vec);

class BCX_API base_transfer_common
{
public:
    base_transfer_common(
        bc::blockchain::block_chain_impl& blockchain,
        std::vector<receiver_record>&& receiver_list, uint64_t fee,
        std::string&& symbol, std::string&& from, std::string&& mychange)
        : blockchain_{blockchain}
        , symbol_{symbol}
        , from_{from}
        , mychange_{mychange}
        , payment_etp_{fee}
        , receiver_list_{receiver_list}
    {
    };

    virtual ~base_transfer_common()
    {
        receiver_list_.clear();
        from_list_.clear();
    };

    static const uint64_t maximum_fee{10000000000};
    static const uint64_t minimum_fee{10000};
    static const uint64_t tx_limit{677};
    static const uint64_t attach_version{1};

    virtual bool get_spendable_output(chain::output&, const chain::history&, uint64_t height) const;
    virtual void sync_fetchutxo(const std::string& prikey, const std::string& addr);
    virtual void sum_payment_amount();
    virtual void populate_change();

    // common functions, single responsibility.
    static void check_fee_in_valid_range(uint64_t fee);
    void sum_payments();
    void check_receiver_list_not_empty() const;
    void populate_etp_change(const std::string& addr = std::string(""));
    void populate_asset_change(const std::string& addr);
    void populate_asset_cert_change(const std::string& addr);

protected:
    bc::blockchain::block_chain_impl& blockchain_;
    tx_type                           tx_; // target transaction
    std::string                       symbol_;
    std::string                       from_;
    std::string                       mychange_;
    uint64_t                          tx_item_idx_{0};
    uint64_t                          payment_etp_{0};
    uint64_t                          payment_asset_{0};
    uint64_t                          payment_asset_cert_{asset_cert_ns::none};
    uint64_t                          unspent_etp_{0};
    uint64_t                          unspent_asset_{0};
    uint64_t                          unspent_asset_cert_{asset_cert_ns::none};
    std::vector<receiver_record>      receiver_list_;
    std::vector<address_asset_record> from_list_;
};

class BCX_API base_transfer_helper : public base_transfer_common
{
public:
    base_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::vector<receiver_record>&& receiver_list,
        uint64_t fee, std::string&& symbol = std::string(""),
        std::string&& mychange = std::string("")):
        base_transfer_common(blockchain, std::move(receiver_list), fee,
                std::move(symbol), std::move(from), std::move(mychange)),
        cmd_{cmd},
        name_{name},
        passwd_{passwd}
    {
    };

    ~base_transfer_helper()
    {
    }

    virtual void populate_unspent_list();

    virtual void populate_tx_header() {
        tx_.version = transaction_version::check_output_script;
        tx_.locktime = 0;
    };

    virtual void populate_tx_inputs();
    virtual attachment populate_output_attachment(receiver_record& record);
    virtual void populate_tx_outputs();
    virtual void check_tx();
    virtual void sign_tx_inputs();
    void send_tx();
    virtual void exec();
    tx_type& get_transaction();
    std::vector<unsigned char> satoshi_to_chunk(const int64_t& value);

protected:
    command&                          cmd_;
    std::string                       name_;
    std::string                       passwd_;
};

class BCX_API base_transaction_constructor : public base_transfer_common
{
public:
    base_transaction_constructor(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, std::vector<receiver_record>&& receiver_list,
        std::string&& symbol, std::string&& mychange,
        std::string&& message, uint64_t fee):
        base_transfer_common(blockchain, std::move(receiver_list), fee,
                std::move(symbol), "", std::move(mychange)),
        type_{type},
        message_{message},
        from_vec_{from_vec}
    {
    };

    virtual ~base_transaction_constructor()
    {
        from_vec_.clear();
    };

    virtual void sum_payment_amount();
    virtual void populate_unspent_list();
    virtual void populate_change();

    virtual void populate_tx_header() {
        tx_.version = transaction_version::check_output_script;
        tx_.locktime = 0;
    };

    virtual void populate_tx_inputs();
    virtual attachment populate_output_attachment(receiver_record& record);
    virtual void populate_tx_outputs();
    virtual void check_tx();
    virtual void exec();
    tx_type& get_transaction();
    std::vector<unsigned char> satoshi_to_chunk(const int64_t& value);

protected:
    utxo_attach_type                  type_{utxo_attach_type::invalid};
    std::string                       message_;
    std::vector<std::string>          from_vec_; // from address vector
};


class BCX_API depositing_etp : public base_transfer_helper
{
public:
    depositing_etp(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& to, std::vector<receiver_record>&& receiver_list,
        uint16_t deposit_cycle = 7, uint64_t fee = 10000):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
                std::string(""), std::move(receiver_list), fee),
        to_{to}, deposit_cycle_{deposit_cycle}
        {};

    ~depositing_etp(){};

    static const std::vector<uint16_t> vec_cycle;

    uint32_t get_reward_lock_height();
    // modify lock script
    void populate_tx_outputs() override ;

private:
    std::string                       to_;
    uint16_t                          deposit_cycle_{7}; // 7 days
};

class BCX_API depositing_etp_transaction : public base_transaction_constructor
{
public:
    depositing_etp_transaction(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, std::vector<receiver_record>&& receiver_list,
        uint16_t deposit, std::string&& mychange,
        std::string&& message, uint64_t fee):
        base_transaction_constructor(blockchain, type, std::move(from_vec),
            std::move(receiver_list), std::string(""),
            std::move(mychange), std::move(message), fee),
            deposit_{deposit}
        {};

    ~depositing_etp_transaction(){};

    static const std::vector<uint16_t> vec_cycle;

    uint32_t get_reward_lock_height();
    // modify lock script
    void populate_tx_outputs() override ;

private:
    uint16_t                          deposit_{7}; // 7 days
};


class BCX_API sending_etp : public base_transfer_helper
{
public:
    sending_etp(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::vector<receiver_record>&& receiver_list, uint64_t fee):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
                std::move(from), std::move(receiver_list), fee)
        {};

    ~sending_etp(){};
};

class BCX_API sending_etp_more : public base_transfer_helper
{
public:
    sending_etp_more(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::vector<receiver_record>&& receiver_list,
        std::string&& mychange, uint64_t fee):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
                std::move(from), std::move(receiver_list), fee, "", std::move(mychange))
        {};

    ~sending_etp_more(){};
};

class BCX_API sending_multisig_etp : public base_transfer_helper
{
public:
    sending_multisig_etp(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::vector<receiver_record>&& receiver_list, uint64_t fee,
        account_multisig& multisig):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
                std::move(from), std::move(receiver_list), fee),
        multisig_{multisig}
        {};

    ~sending_multisig_etp(){};

    void populate_unspent_list() override;
    void sign_tx_inputs() override ;
    void exec() override;
private:
    account_multisig multisig_;
};

class BCX_API issuing_asset : public base_transfer_helper
{
public:
    issuing_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        std::vector<receiver_record>&& receiver_list, uint64_t fee):
        base_transfer_helper(cmd, blockchain,
                std::move(name), std::move(passwd),
                std::move(from), std::move(receiver_list),
                fee, std::move(symbol)),
        attenuation_model_param(std::move(model_param))
        {};

    ~issuing_asset(){};
    void sum_payment_amount() override;
    void populate_tx_outputs() override;
    void populate_change() override;
    void populate_unspent_list() override;
    void sync_fetchutxo (const std::string& prikey, const std::string& addr) override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };

private:
    std::string attenuation_model_param;
};

class BCX_API secondary_issuing_asset : public base_transfer_helper
{
public:
    secondary_issuing_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        std::vector<receiver_record>&& receiver_list, uint64_t fee, uint64_t volume):
        base_transfer_helper(cmd, blockchain,
                std::move(name), std::move(passwd),
                std::move(from), std::move(receiver_list),
                fee, std::move(symbol)),
        volume_(volume),
        attenuation_model_param(std::move(model_param))
    {};

    ~secondary_issuing_asset(){};
    void sum_payment_amount() override;
    void populate_change() override;
    void populate_unspent_list() override;
    void sync_fetchutxo (const std::string& prikey, const std::string& addr) override;
    attachment populate_output_attachment(receiver_record& record) override;
    void populate_tx_outputs() override;

    uint64_t get_volume() { return volume_; };
    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };

private:
    uint64_t volume_;
    std::shared_ptr<asset_detail> issued_asset_;
    std::string attenuation_model_param;
};

class BCX_API sending_asset : public base_transfer_helper
{
public:
    sending_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list),
            fee, std::move(symbol))
        {};

    ~sending_asset(){};

    void populate_change() override;
};

class BCX_API issuing_did : public base_transfer_helper
{
public:
    issuing_did(command& cmd, bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list),
            fee, std::move(symbol))
        {};

    ~issuing_did(){};

    void sum_payment_amount() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };
};

class BCX_API sending_did : public base_transfer_helper
{
public:
    sending_did(command& cmd, bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd,
        std::string&& from,std::string&& feefrom, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee):
        base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list),
            fee, std::move(symbol)),fromfee(feefrom)
        {

        };

    ~sending_did(){};
    void sync_fetchutxo (const std::string& prikey, const std::string& addr) override;
    void populate_change() override;
    std::string fromfee ;
};

class BCX_API transferring_asset_cert : public base_transfer_helper
{
public:
    transferring_asset_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::vector<receiver_record>&& receiver_list, uint64_t fee):
        base_transfer_helper(cmd, blockchain,
            std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list),
            fee, std::move(symbol))
        {};

    ~transferring_asset_cert(){};

    void sum_payment_amount() override;
    void populate_change() override;
    void sync_fetchutxo (const std::string& prikey, const std::string& addr) override;
};


} // commands
} // explorer
} // libbitcoin
