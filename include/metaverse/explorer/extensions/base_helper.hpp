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
/// NOTICE: createrawtx / createmultisigtx --type option is using these values.
/// DO NOT CHANGE EXIST ITEMS!!!
enum class utxo_attach_type : uint32_t
{
    etp = 0,
    deposit = 1,
    asset_issue = 2,
    asset_transfer = 3,
    unused1 = 4,
    asset_locked_transfer = 5,
    message = 6,
    asset_cert = 7,
    asset_secondaryissue = 8,
    did_issue = 9,
    did_transfer = 10,
    asset_cert_issue = 11,
    asset_cert_transfer = 12,
    asset_cert_autoissue = 13,
    invalid = 0xffffffff
};

extern utxo_attach_type get_utxo_attach_type(const chain::output&);

struct address_asset_record
{
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

struct receiver_record
{
    typedef std::vector<receiver_record> list;

    std::string target;
    std::string symbol;
    uint64_t    amount{0}; // etp value
    uint64_t    asset_amount{0};
    asset_cert_type asset_cert{asset_cert_ns::none};

    utxo_attach_type type{utxo_attach_type::invalid};
    attachment attach_elem;  // used for MESSAGE_TYPE, used for information transfer etc.
    chain::input_point input_point{null_hash, max_uint32};

    receiver_record()
        : target()
        , symbol()
        , amount(0)
        , asset_amount(0)
        , asset_cert(asset_cert_ns::none)
        , type(utxo_attach_type::invalid)
        , attach_elem()
        , input_point{null_hash, max_uint32}
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t asset_amount_,
        utxo_attach_type type_, const attachment& attach_elem_ = attachment(),
        const chain::input_point& input_point_ = {null_hash, max_uint32})
        : receiver_record(target_, symbol_, amount_, asset_amount_,
            asset_cert_ns::none, type_, attach_elem_, input_point_)
    {}

    receiver_record(const std::string& target_, const std::string& symbol_,
        uint64_t amount_, uint64_t asset_amount_, asset_cert_type asset_cert_,
        utxo_attach_type type_, const attachment& attach_elem_ = attachment(),
        const chain::input_point& input_point_ = {null_hash, max_uint32})
        : target(target_)
        , symbol(symbol_)
        , amount(amount_)
        , asset_amount(asset_amount_)
        , asset_cert(asset_cert_)
        , type(type_)
        , attach_elem(attach_elem_)
        , input_point(input_point_)
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
void sync_fetchbalance(wallet::payment_address& address,
    bc::blockchain::block_chain_impl& blockchain, balances& addr_balance);

void sync_fetch_asset_balance(const std::string& address, bool sum_all,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<asset_balances::list> sh_asset_vec);

void sync_fetch_asset_cert_balance(const std::string& address, const string& symbol,
    bc::blockchain::block_chain_impl& blockchain,
    std::shared_ptr<asset_cert::list> sh_vec, asset_cert_type cert_type=asset_cert_ns::none);

std::string get_random_payment_address(std::shared_ptr<std::vector<account_address>>,
    bc::blockchain::block_chain_impl& blockchain);

std::string get_address_from_did(const std::string& did,
    bc::blockchain::block_chain_impl& blockchain);

void check_asset_symbol(const std::string& symbol, bool check_sensitive=false);
void check_did_symbol(const std::string& symbol,  bool check_sensitive=false);

bool is_nova_feature_activated(const bc::blockchain::block_chain_impl& blockchain);

class BCX_API base_transfer_common
{
public:
    enum filter : uint8_t {
        FILTER_ETP = 1 << 0,
        FILTER_ASSET = 1 << 1,
        FILTER_ASSETCERT = 1 << 2,
        FILTER_DID = 1 << 3,
        FILTER_ALL = 0xff,
        // if specify 'from_' address,
        // then get these types' unspent only from 'from_' address
        FILTER_PAYFROM = FILTER_ETP | FILTER_ASSET,
        FILTER_ALL_BUT_PAYFROM = FILTER_ALL & ~FILTER_PAYFROM
    };

    base_transfer_common(
        bc::blockchain::block_chain_impl& blockchain,
        receiver_record::list&& receiver_list, uint64_t fee,
        std::string&& symbol, std::string&& from, std::string&& mychange)
        : blockchain_{blockchain}
        , symbol_{std::move(symbol)}
        , from_{std::move(from)}
        , mychange_{std::move(mychange)}
        , payment_etp_{fee}
        , receiver_list_{std::move(receiver_list)}
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
    virtual chain::operation::stack get_script_operations(const receiver_record& record) const;
    virtual void sync_fetchutxo(
            const std::string& prikey, const std::string& addr, filter filter = FILTER_ALL);
    virtual attachment populate_output_attachment(const receiver_record& record);
    virtual void sum_payments();
    virtual void sum_payment_amount();
    virtual void populate_change();
    virtual void populate_tx_outputs();
    virtual void populate_unspent_list() = 0;
    virtual void sign_tx_inputs();
    virtual void send_tx();
    virtual void populate_tx_header();

    // common functions, single responsibility.
    static void check_fee_in_valid_range(uint64_t fee);
    void check_receiver_list_not_empty() const;
    void populate_etp_change(const std::string& address = std::string(""));
    void populate_asset_change(const std::string& address = std::string(""));
    bool is_payment_satisfied(filter filter = FILTER_ALL) const;
    void check_payment_satisfied(filter filter = FILTER_ALL) const;
    void populate_tx_inputs();
    void check_tx();
    void exec();

    std::string get_mychange_address(filter filter) const;

    tx_type& get_transaction() { return tx_; }
    const tx_type& get_transaction() const { return tx_; }

    // in secondary issue, locked asset can also verify threshold condition
    virtual bool is_locked_asset_as_payment() const {return false;}

    virtual bool filter_out_address(const std::string& address) const;

    virtual std::string get_sign_tx_multisig_script(const address_asset_record& from) const;

    void set_did_verify_attachment(const receiver_record& record, attachment& attach);

protected:
    bc::blockchain::block_chain_impl& blockchain_;
    tx_type                           tx_; // target transaction
    std::string                       symbol_;
    std::string                       from_;
    std::string                       mychange_;
    uint64_t                          tx_item_idx_{0};
    uint64_t                          payment_etp_{0};
    uint64_t                          payment_asset_{0};
    uint64_t                          unspent_etp_{0};
    uint64_t                          unspent_asset_{0};
    std::vector<asset_cert_type>      payment_asset_cert_;
    std::vector<asset_cert_type>      unspent_asset_cert_;
    bool                              payment_did_flag{false};
    bool                              unspent_did_flag{false};
    std::vector<receiver_record>      receiver_list_;
    std::vector<address_asset_record> from_list_;
};

class BCX_API base_transfer_helper : public base_transfer_common
{
public:
    base_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        uint64_t fee, std::string&& symbol = std::string(""),
        std::string&& mychange = std::string(""))
        : base_transfer_common(blockchain, std::move(receiver_list), fee,
            std::move(symbol), std::move(from),
            std::move(mychange))
        , cmd_{cmd}
        , name_{std::move(name)}
        , passwd_{std::move(passwd)}
    {}

    ~base_transfer_helper()
    {}

    void populate_unspent_list() override;

protected:
    command&                          cmd_;
    std::string                       name_;
    std::string                       passwd_;
};

class BCX_API base_multisig_transfer_helper : public base_transfer_helper
{
public:
    base_multisig_transfer_helper(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        uint64_t fee, std::string&& symbol,
        account_multisig&& multisig_from)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , multisig_(std::move(multisig_from))
    {}

    ~base_multisig_transfer_helper()
    {}

    bool filter_out_address(const std::string& address) const override;

    std::string get_sign_tx_multisig_script(const address_asset_record& from) const override;

    void send_tx() override;

protected:
    // for multisig address
    account_multisig multisig_;
};

class BCX_API base_transaction_constructor : public base_transfer_common
{
public:
    base_transaction_constructor(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, receiver_record::list&& receiver_list,
        std::string&& symbol, std::string&& mychange,
        std::string&& message, uint64_t fee)
        : base_transfer_common(blockchain, std::move(receiver_list), fee,
            std::move(symbol), "", std::move(mychange))
        , type_{type}
        , message_{std::move(message)}
        , from_vec_{std::move(from_vec)}
    {}

    virtual ~base_transaction_constructor()
    {
        from_vec_.clear();
    };

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    // no operation in exec
    void sign_tx_inputs() override {}
    void send_tx() override {}

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
        std::string&& to, receiver_record::list&& receiver_list,
        uint16_t deposit_cycle = 7, uint64_t fee = 10000)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::string(""), std::move(receiver_list), fee)
        , to_{std::move(to)}
        , deposit_cycle_{deposit_cycle}
    {}

    ~depositing_etp(){}

    static const std::vector<uint16_t> vec_cycle;

    uint32_t get_reward_lock_height() const;

    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::string                       to_;
    uint16_t                          deposit_cycle_{7}; // 7 days
};

class BCX_API depositing_etp_transaction : public base_transaction_constructor
{
public:
    depositing_etp_transaction(bc::blockchain::block_chain_impl& blockchain, utxo_attach_type type,
        std::vector<std::string>&& from_vec, receiver_record::list&& receiver_list,
        uint16_t deposit, std::string&& mychange,
        std::string&& message, uint64_t fee)
        : base_transaction_constructor(blockchain, type, std::forward<std::vector<std::string>>(from_vec),
            std::move(receiver_list), std::string(""),
            std::move(mychange), std::move(message), fee)
        , deposit_{deposit}
    {}

    ~depositing_etp_transaction(){}

    static const std::vector<uint16_t> vec_cycle;

    uint32_t get_reward_lock_height() const;

    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    uint16_t                          deposit_{7}; // 7 days
};

class BCX_API sending_etp : public base_transfer_helper
{
public:
    sending_etp(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee)
    {}

    ~sending_etp(){}
};

class BCX_API sending_etp_more : public base_transfer_helper
{
public:
    sending_etp_more(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list,
        std::string&& mychange, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, "", std::move(mychange))
    {}

    ~sending_etp_more(){}
};

class BCX_API sending_multisig_tx : public base_multisig_transfer_helper
{
public:
    sending_multisig_tx(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, receiver_record::list&& receiver_list, uint64_t fee,
        account_multisig& multisig, std::string&& symbol = std::string(""))
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig))
    {}

    ~sending_multisig_tx(){}

    void populate_change() override;
};

class BCX_API issuing_asset : public base_transfer_helper
{
public:
    issuing_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , attenuation_model_param_{std::move(model_param)}
    {}

    ~issuing_asset(){}

    void sum_payments() override;
    void sum_payment_amount() override;
    attachment populate_output_attachment(const receiver_record& record) override;

    chain::operation::stack get_script_operations(const receiver_record& record) const override;

private:
    std::shared_ptr<asset_detail> unissued_asset_;
    std::string domain_cert_address_;
    std::string attenuation_model_param_;
};

class BCX_API secondary_issuing_asset : public base_transfer_helper
{
public:
    secondary_issuing_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        std::string&& model_param,
        receiver_record::list&& receiver_list, uint64_t fee, uint64_t volume)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , volume_(volume)
        , attenuation_model_param_{std::move(model_param)}
    {}

    ~secondary_issuing_asset(){}

    void sum_payment_amount() override;
    void populate_change() override;
    attachment populate_output_attachment(const receiver_record& record) override;
    chain::operation::stack get_script_operations(const receiver_record& record) const override;

    uint64_t get_volume() { return volume_; };

    bool is_locked_asset_as_payment() const override {return true;}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };

private:
    uint64_t volume_{0};
    std::shared_ptr<asset_detail> issued_asset_;
    std::string target_address_;
    std::string attenuation_model_param_;
};

class BCX_API sending_asset : public base_transfer_helper
{
public:
    sending_asset(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
    {}

    ~sending_asset()
    {}

    void populate_change() override;
};

class BCX_API issuing_did : public base_multisig_transfer_helper
{
public:
    issuing_did(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol, receiver_record::list&& receiver_list, uint64_t fee,
        account_multisig&& multisig)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig))
    {}

    ~issuing_did()
    {}

    void sum_payment_amount() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };
};

class BCX_API sending_multisig_did : public base_transfer_helper
{
public:
    sending_multisig_did(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& feefrom, std::string&& symbol,
        receiver_record::list&& receiver_list
        , uint64_t fee, account_multisig&& multisig, account_multisig&& multisigto)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
        , fromfee(feefrom), multisig_from_(std::move(multisig)), multisig_to_(std::move(multisigto))
    {}

    ~sending_multisig_did()
    {}

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    std::string get_sign_tx_multisig_script(const address_asset_record& from) const override;

    // no operation in exec
    void send_tx() override {}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };

private:
    std::string fromfee;
    account_multisig multisig_from_;
    account_multisig multisig_to_;
};

class BCX_API sending_did : public base_transfer_helper
{
public:
    sending_did(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& feefrom, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol)),fromfee(feefrom)
    {}

    ~sending_did()
    {}

    void sum_payment_amount() override;
    void populate_unspent_list() override;
    void populate_change() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };

private:
    std::string fromfee;
};

class BCX_API transferring_asset_cert : public base_multisig_transfer_helper
{
public:
    transferring_asset_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee,
        account_multisig&& multisig_from)
        : base_multisig_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol),
            std::move(multisig_from))
    {}

    ~transferring_asset_cert()
    {}

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };
};

class BCX_API issuing_asset_cert : public base_transfer_helper
{
public:
    issuing_asset_cert(command& cmd, bc::blockchain::block_chain_impl& blockchain,
        std::string&& name, std::string&& passwd,
        std::string&& from, std::string&& symbol,
        receiver_record::list&& receiver_list, uint64_t fee)
        : base_transfer_helper(cmd, blockchain, std::move(name), std::move(passwd),
            std::move(from), std::move(receiver_list), fee, std::move(symbol))
    {}

    ~issuing_asset_cert()
    {}

    void sum_payment_amount() override;

    void populate_tx_header() override {
        tx_.version = transaction_version::check_nova_feature;
        tx_.locktime = 0;
    };
};


} // commands
} // explorer
} // libbitcoin
