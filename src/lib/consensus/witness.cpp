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
uint32_t witness::epoch_cycle_height = 10000;
uint32_t witness::vote_maturity = 12;
uint32_t witness::max_dpos_interval = 3; // seconds

witness* witness::instance_ = nullptr;

witness::witness(p2p_node& node)
    : node_(node)
    , setting_(node_.chain_impl().chain_settings())
    , witness_list_()
    , candidate_list_()
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
        witness::vote_maturity = 2;
        witness::max_dpos_interval = 3;
    }

#ifdef PRIVATE_CHAIN
    witness::pow_check_point_height = 100;
    witness::witness_enable_height = 10;
    witness::witness_number = 11;
    witness::epoch_cycle_height = 100;
    witness::vote_maturity = 2;
    witness::max_dpos_interval = 1;
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

witness::list witness::get_candidate_list() const
{
    shared_lock lock(mutex_);
    return candidate_list_;
}

void witness::swap_witness_list(list& l)
{
    unique_lock ulock(mutex_);
    witness_list_.swap(l);
}

void witness::swap_candidate_list(list& l)
{
    candidate_list_.swap(l);
}

bool witness::is_witness(const witness_id& id) const
{
    shared_lock lock(mutex_);
    return (!id.empty()) && exists(witness_list_, id);
}

bool witness::register_witness(const witness_id& id)
{
    upgrade_lock lock(mutex_);

    if (exists(witness_list_, id) || exists(candidate_list_, id)) {
        log::debug(LOG_HEADER) << "In register_witness, " << id << " is already registered.";
        return false;
    }

    upgrade_to_unique_lock ulock(lock);
    candidate_list_.push_back(id);
    return true;
}

bool witness::unregister_witness(const witness_id& id)
{
    upgrade_lock lock(mutex_);

    auto witness_pos = finds(witness_list_, id);
    auto candidate_pos = finds(candidate_list_, id);

    if (witness_pos == witness_list_.end() && candidate_pos == candidate_list_.end()) {
        log::debug(LOG_HEADER) << "In unregister_witness, " << id << " is not registered.";
        return false;
    }

    upgrade_to_unique_lock ulock(lock);
    if (witness_pos != witness_list_.end()) {
        // clear data instead of remove, as a stub
        *witness_pos = witness_id();
    }
    if (candidate_pos != candidate_list_.end()) {
        candidate_list_.erase(candidate_pos);
    }
    return true;
}

std::string witness::show_list() const
{
    shared_lock lock(mutex_);
    std::string res;
    res += "witness : [";
    for (const auto& witness : witness_list_) {
        res += witness_to_string(witness) + "   ";
    }
    res += "] ";

    res += "candidate : [";
    for (const auto& candidate : candidate_list_) {
        res += witness_to_string(candidate) + "   ";
    }
    res += "]";
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

    for (const auto& candidate : candidate_list_) {
        ops.push_back({chain::opcode::special, witness_to_public_key(candidate)});
    }

    return output;
}

// DPOS_TODO: add algorithm to generate the witness list of each epoch
bool witness::calc_witness_list(uint64_t height)
{
    unique_lock ulock(mutex_);

    witness_list_.clear();
    candidate_list_.clear();

#ifdef PRIVATE_CHAIN
    std::vector<std::string> initial_witness_list = {
        "033b8ffd5f5b1ff1aed54cac72853056ffe66c4b6ce314f80352a97c75a7d62bc8",
        "0226515661e5ba01aa211048c7d34e1e6d577b7460de9582bfd8f5c70b81f07e87",
        "0270e8ab22af412d698ae51b92764914918747d6e752539cba997b9585238167ed",
        "026789edf6edceeed4d11d87ccb2bc0d626b1c407cd3a455bfb0bc250599caf912",
        "034758025dd5885ccdcf01aaf4ad3af7e3b17306bcc6c141cb10cfd684fc8a9a3c",
        "02a9b1c925f6fd140359d559299ba38c131dde6ad33b5fdd818cccfebdf66a4da3",
        "03cb25bc68f807d09656103c1e9635f5c4ce5a34b4b08a11e783c2033c0330024d",
        "02cb2cdccfa8f31ffe8b486aa9e12858f8754b438d2001ff2a9bb72427b23e5dc0",
        "03d27296d98a84d2abe8879fd101252fb9ef30695f763591396406d1a12cb83921",
        "0328ee9d3b3adc8c687d758f63a663e242005f1df7e18a484149af15a21f4addda",
        "02f6bd99f7529420d45b8d1a16459e11d61b8ded7cf55d7654d43ebab7f307ba3b"
    };

    for (const auto& id : initial_witness_list) {
        witness_list_.emplace_back(to_chunk(id));
    }
#endif

    if (witness_list_.size() != witness_number) {
#ifdef PRIVATE_CHAIN
    log::error(LOG_HEADER)
        << "calc witness list failed at height " << height
        << ", result is " << show_list();
#endif
        return false;
    }

    auto random_fun = [](size_t i) {return pseudo_random(0, i);};
    std::random_shuffle(witness_list_.begin(), witness_list_.end(), random_fun);

    return true;
}

bool witness::verify_vote_result(const chain::block& block) const
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
            return false;
        }
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

bool witness::update_witness_list(uint64_t height)
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
    return update_witness_list(*sp_block);
}

bool witness::update_witness_list(const chain::block& block)
{
    if (!verify_vote_result(block)) {
#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
        << "verify vote result failed at height " << block.header.number;
#endif
        return false;
    }

    unique_lock ulock(mutex_);
    witness_list_.clear();
    candidate_list_.clear();

    const auto& transactions = block.transactions;
    auto& ops = transactions.front().outputs.back().script.operations;
    BITCOIN_ASSERT_MSG(ops.size() >= witness_number, "wrong number of vote result.");

    size_t i = 0;
    for (; i < witness_number; ++i) {
        witness_list_.emplace_back(to_witness_id(chain::operation::factory_from_data(ops[i].to_data()).data));
    }
    for (; i < ops.size(); ++i) {
        candidate_list_.emplace_back(to_witness_id(chain::operation::factory_from_data(ops[i].to_data()).data));
    }
    return true;
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

bool witness::is_witness_prepared() const
{
    return !witness_list_.empty();
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
