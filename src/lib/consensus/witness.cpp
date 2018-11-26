/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/consensus/witness.hpp>
#include <metaverse/macros_define.hpp>
#include <metaverse/node/p2p_node.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <future>

#define LOG_HEADER "Witness"

namespace libbitcoin {
namespace consensus {

uint32_t witness::pow_check_point_height = 100;
uint64_t witness::witness_enable_height = 2000000;
uint32_t witness::witness_number = 11;
uint32_t witness::epoch_cycle_height = 20000;
uint32_t witness::register_witness_lock_height = 10000;
uint64_t witness::witness_lock_threshold = 1000*(1e8); // ETP bits
uint32_t witness::vote_maturity = 12;
uint32_t witness::max_dpos_interval = 3; // seconds

witness* witness::instance_ = nullptr;

const uint32_t witness::witness_register_fee = 123456789;
const std::string witness::witness_registry_did = "witness_registry";

static const std::string stub_public_key = "0400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

witness::witness(p2p_node& node)
    : node_(node)
    , setting_(node_.chain_impl().chain_settings())
    , witness_list_()
    , mutex_()
{
}

witness::~witness()
{
}

void witness::init(p2p_node& node)
{
    static witness s_instance(node);
    instance_ = &s_instance;

    if (instance_->setting_.use_testnet_rules) {
        witness::pow_check_point_height = 100;
        witness::witness_enable_height = 1000000;
        witness::witness_number = 5;
        witness::epoch_cycle_height = 1000;
        witness::register_witness_lock_height = 500;
        witness::witness_lock_threshold = 10*(1e8); // ETP bits
        witness::vote_maturity = 2;
        witness::max_dpos_interval = 3;
    }

#ifdef PRIVATE_CHAIN
    witness::pow_check_point_height = 100;
    witness::witness_enable_height = 50;
    witness::witness_number = 11;
    witness::epoch_cycle_height = 100;
    witness::register_witness_lock_height = 50;
    witness::witness_lock_threshold = 1*(1e8); // ETP bits
    witness::vote_maturity = 2;
    witness::max_dpos_interval = 1;
#endif

#ifdef PRIVATE_CHAIN
    log::info("blockchain") << "running prinet";
#else
    log::info("blockchain")
        << (!node.is_use_testnet_rules() ? "running mainnet" : "running testnet");
#endif
}

witness& witness::create(p2p_node& node)
{
    static unique_mutex umutex;
    if (!instance_) {
        scoped_lock l(umutex);
        init(node);
    }
    return *instance_;
}

witness& witness::get()
{
    BITCOIN_ASSERT_MSG(instance_, "use witness::create() before witness::get()");
    return *instance_;
}

witness::iterator witness::finds(list& l, const witness_id& id)
{
    auto cmp = [&id](const witness_id& item){return id == item;};
    return std::find_if(std::begin(l), std::end(l), cmp);
}

witness::const_iterator witness::finds(const list& l, const witness_id& id)
{
    auto cmp = [&id](const witness_id& item){return id == item;};
    return std::find_if(std::begin(l), std::end(l), cmp);
}

bool witness::exists(const list& l, const witness_id& id)
{
    return finds(l, id) != l.end();
}

witness::list witness::get_witness_list() const
{
    shared_lock lock(mutex_);
    return witness_list_;
}

void witness::swap_witness_list(list& l)
{
    unique_lock ulock(mutex_);
    witness_list_.swap(l);
}

bool witness::is_witness(const witness_id& id) const
{
    shared_lock lock(mutex_);
    return (!id.empty()) && exists(witness_list_, id);
}

std::string witness::show_list() const
{
    shared_lock lock(mutex_);
    return show_list(witness_list_);
}

std::string witness::show_list(const list& witness_list)
{
    std::string res;
    res += "witness : [";
    for (const auto& witness : witness_list) {
        res += witness_to_string(witness) + "   ";
    }
    res += "] ";
    return res;
}

chain::output witness::create_witness_vote_result(uint64_t height)
{
    chain::output output;
    output.value = 0;
    if (!calc_witness_list(height)) {
        return output;
    }

    shared_lock lock(mutex_);

    auto& ops = output.script.operations;
    for (const auto& witness : witness_list_) {
        ops.push_back({chain::opcode::special, witness_to_public_key(witness)});
    }

    return output;
}

bool witness::calc_witness_list(uint64_t height)
{
    list witness_list;
    if (!calc_witness_list(witness_list, height)) {
        return false;
    }

    swap_witness_list(witness_list);
    return true;
}

bool witness::calc_witness_list(list& witness_list, uint64_t height) const
{
    if (!is_begin_of_epoch(height)) {
#ifdef PRIVATE_CHAIN
        log::error(LOG_HEADER) << "calc witness list must at the begin of epoch, not " << height;
#endif
        return false;
    }

    witness_list.clear();

    auto& chain = const_cast<blockchain::block_chain_impl&>(node_.chain_impl());
    auto did_detail = chain.get_registered_did(witness::witness_registry_did);
    if (!did_detail) {
        return false;
    }

    using addr_pubkey_pair_t = std::pair<std::string, public_key_t>;
    using locked_balance_t = std::pair<uint64_t, uint64_t>;
    using locked_record_t = std::pair<addr_pubkey_pair_t, locked_balance_t>;
    auto cmp_locked_record =
        [](const locked_record_t& r1, const locked_record_t& r2){
            return r1.second > r2.second; // descend
        };

    auto from_height = (height > epoch_cycle_height) ? (height - epoch_cycle_height + 1) : 1;
    auto register_addresses = chain.get_register_witnesses(did_detail->get_address(), from_height);

    for (const auto& prev_witness_id : get_witness_list()) {
        auto pub_key = witness_to_public_key(prev_witness_id);
        if (std::find_if(register_addresses.begin(), register_addresses.end(),
                [&pub_key](const addr_pubkey_pair_t& item){
                    return item.second == pub_key;
                }) != register_addresses.end()) {
            continue;
        }
        auto pay_address = wallet::ec_public(pub_key).to_payment_address();
        auto address = pay_address.encoded();
        register_addresses.emplace_back(std::make_pair(address, pub_key));
    }

    std::vector<locked_record_t> statistics;
    for (const auto& addr_pubkey : register_addresses) {
        auto locked_balance = chain.get_locked_balance(addr_pubkey.first, height + register_witness_lock_height);
        if (locked_balance.first < witness_lock_threshold) {
            continue;
        }
        statistics.push_back(std::make_pair(addr_pubkey, locked_balance));
    }

    std::sort(statistics.begin(), statistics.end(), cmp_locked_record);
    if (statistics.size() > witness_number) {
        statistics.resize(witness_number);
    }

    for (const auto& record : statistics) {
        witness_list.emplace_back(to_witness_id(record.first.second));
    }

    if (witness_list.size() < witness_number) {
        witness_list.resize(witness_number, to_chunk(stub_public_key));
    }

    pseudo_random::shuffle(witness_list);

    return true;
}

bool witness::verify_vote_result(const chain::block& block, list& witness_list, bool calc) const
{
    const auto& transactions = block.transactions;
    if (transactions.size() != 1) {
        return false;
    }
    if (transactions.front().outputs.size() != 2) {
        return false;
    }
    auto& ops = transactions.front().outputs.back().script.operations;
    if ((ops.size() < witness_number) || !chain::operation::is_push_only(ops)) {
        return false;
    }
    for (const auto& op : ops) {
        if (!is_public_key(chain::operation::factory_from_data(op.to_data()).data)) {
#ifdef PRIVATE_CHAIN
            log::info(LOG_HEADER)
                << "in verify_vote_result ops is not public key, height " << block.header.number;
#endif
            return false;
        }
    }

    for (size_t i = 0; i < witness_number; ++i) {
        witness_list.emplace_back(to_witness_id(chain::operation::factory_from_data(ops[i].to_data()).data));
    }

    if (!calc) {
        return true;
    }

    list calced_witness_list;
    if (!calc_witness_list(calced_witness_list, block.header.number)) {
#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
        << "in verify_vote_result -> calc_witness_list failed, height " << block.header.number;
#endif
        return false;
    }

    if (std::set<witness_id>(witness_list.begin(), witness_list.end()) !=
        std::set<witness_id>(calced_witness_list.begin(), calced_witness_list.end())) {
#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
        << "in verify_vote_result compare calc_witness_list result failed, height " << block.header.number;
#endif
        return false;
    }

    return true;
}

chain::block::ptr witness::fetch_vote_result_block(uint64_t height)
{
    auto vote_height = get_vote_result_height(height);
    if (vote_height == 0) {
        return nullptr;
    }

    std::promise<code> p;
    chain::block::ptr sp_block;

    node_.chain_impl().fetch_block(vote_height,
        [&p, &sp_block](const code & ec, chain::block::ptr block){
            if (ec) {
                p.set_value(ec);
                return;
            }
            sp_block = block;
            p.set_value(error::success);
        });

    auto result = p.get_future().get();
    if (result) {
        return nullptr;
    }
    return sp_block;
}

bool witness::update_witness_list(uint64_t height, bool calc)
{
    if (!is_witness_enabled(height)) {
        return true;
    }
    auto sp_block = fetch_vote_result_block(height);
    if (!sp_block) {
#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
        << "fetch vote result block failed at height " << height;
#endif
        return false;
    }
    return update_witness_list(*sp_block, calc);
}

bool witness::update_witness_list(const chain::block& block, bool calc)
{
    list witness_list;
    if (!verify_vote_result(block, witness_list, calc)) {
#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
        << "verify vote result failed at height " << block.header.number;
#endif
        return false;
    }

    swap_witness_list(witness_list);
    return true;
}

const witness::witness_id& witness::get_witness(uint32_t slot) const
{
    shared_lock lock(mutex_);
    return witness_list_.at(slot);
}

uint32_t witness::get_slot_num(const witness_id& id) const
{
    shared_lock lock(mutex_);
    auto pos = finds(witness_list_, id);
    if (pos != witness_list_.end()) {
        return std::distance(witness_list_.cbegin(), pos);
    }
#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
        << "get slot num failed for " << witness_to_string(id);
#endif
    return max_uint32;
}

witness::public_key_t witness::witness_to_public_key(const witness_id& id)
{
    public_key_t public_key;
    decode_base16(public_key, witness_to_string(id));
    return public_key;
}

std::string witness::witness_to_string(const witness_id& id)
{
    return std::string(std::begin(id), std::end(id));
}

std::string witness::endorse_to_string(const endorsement& endorse)
{
    return encode_base16(endorse);
}

witness::witness_id witness::to_witness_id(const public_key_t& public_key)
{
    return to_chunk(encode_base16(public_key));
}

std::string witness::to_witness_id_str(const public_key_t& public_key)
{
    return encode_base16(public_key);
}

bool witness::sign(endorsement& out, const ec_secret& secret, const chain::header& h)
{
    const auto sighash = h.hash();

    ec_signature signature;
    if (!bc::sign(signature, secret, sighash) || !bc::encode_signature(out, signature)) {
        return false;
    }

    out.push_back(h.number & 0xff);
    return true;
}

bool witness::verify_sign(const endorsement& out, const public_key_t& public_key, const chain::header& h)
{
    if (public_key.empty() || !is_public_key(public_key)) {
#ifdef PRIVATE_CHAIN
        log::error(LOG_HEADER) << "verify witness sign failed, public key is wrong: "
                               << to_witness_id_str(public_key);
#endif
        return false;
    }

    if (out.back() != (h.number & 0xff)) {
#ifdef PRIVATE_CHAIN
        log::error(LOG_HEADER) << "verify witness sign failed, suffix wrong";
#endif
        return false;
    }

    auto distinguished = out;
    distinguished.pop_back();

    ec_signature signature;

    if (!parse_signature(signature, distinguished, true)) {
#ifdef PRIVATE_CHAIN
        log::error(LOG_HEADER) << "verify witness sign failed, parse_signature failed";
#endif
        return false;
    }

    const auto sighash = h.hash();

    if (!bc::verify_signature(public_key, sighash, signature)) {
#ifdef PRIVATE_CHAIN
        log::error(LOG_HEADER)
            << "verify witness signature failed at height " << (h.number + 1)
            << ", public_key is " << to_witness_id_str(public_key)
            << ", signature is " << endorse_to_string(out)
            << ", hash is " << encode_hash(sighash);
#endif
        return false;
    }

    return true;
}

bool witness::verify_signer(const public_key_t& public_key, const chain::block& block, const chain::header& prev_header) const
{
    shared_lock lock(mutex_);
    auto witness_slot_num = get_slot_num(to_witness_id(public_key));
    return verify_signer(witness_slot_num, block, prev_header);
}

bool witness::verify_signer(uint32_t witness_slot_num, const chain::block& block, const chain::header& prev_header) const
{
    if (witness_slot_num >= witness_number) {
        return false;
    }

    const auto& curr_header = block.header;
    auto block_height = curr_header.number;
    auto calced_slot_num = ((block_height - witness_enable_height) % witness_number);

    if (calced_slot_num == witness_slot_num) {
        return true;
    }

    // for safty, the missed  slot should not be mined by the adjacent witnesses
    if (((calced_slot_num + 1) % witness_number == witness_slot_num) ||
        ((witness_slot_num + 1) % witness_number == calced_slot_num)) {
        return false;
    }

    if (get_witness(calced_slot_num) == to_chunk(stub_public_key)) {
        return true;
    }

    // time related logic, compete
    if (curr_header.timestamp > prev_header.timestamp + max_dpos_interval) {
        typedef std::chrono::system_clock wall_clock;
        const auto now_time = wall_clock::now();
        const auto prev_block_time = wall_clock::from_time_t(prev_header.timestamp);
        if (now_time > prev_block_time + std::chrono::seconds(max_dpos_interval)) {
            return true;
        }
    }

    return false;
}

bool witness::is_witness_enabled(uint64_t height)
{
    return height >= witness_enable_height;
}

uint64_t witness::get_height_in_epoch(uint64_t height)
{
    return (height - witness_enable_height) % epoch_cycle_height;
}

// vote result is stored in the beginning of each epoch
uint64_t witness::get_vote_result_height(uint64_t height)
{
    return is_witness_enabled(height) ? (height - get_height_in_epoch(height)) : 0;
}

bool witness::is_begin_of_epoch(uint64_t height)
{
    return is_witness_enabled(height) && get_height_in_epoch(height) == 0;
}

bool witness::is_between_vote_maturity_interval(uint64_t height)
{
    if (!is_witness_enabled(height)) {
        return false;
    }
    // [0 .. vote_maturity)
    if (get_height_in_epoch(height) < vote_maturity) {
        return true;
    }
    // [epoch_cycle_height-vote_maturity .. epoch_cycle_height)
    if (epoch_cycle_height - get_height_in_epoch(height) <= vote_maturity) {
        return true;
    }
    return false;
}

bool witness::is_in_same_epoch(uint64_t height1, uint64_t height2)
{
    return get_vote_result_height(height1) == get_vote_result_height(height2);
}

} // consensus
} // libbitcoin
