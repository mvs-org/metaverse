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
#include <metaverse/node/p2p_node.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <future>
#include <mutex>

#define LOG_HEADER "witness"

namespace libbitcoin {
namespace consensus {

uint32_t witness::pow_check_point_height = 100;
uint64_t witness::witness_enable_height = 2000000;
uint32_t witness::witess_number = 11;
uint32_t witness::epoch_cycle_height = 10000;
uint32_t witness::vote_maturity = 12;

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
        witness::witess_number = 5;
        witness::epoch_cycle_height = 1000;
        witness::vote_maturity = 2;
    }
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

chain::output witness::create_witness_vote_result(uint64_t height)
{
    calc_witness_list(height);

    shared_lock lock(mutex_);

    chain::output output;
    output.value = 0;
    auto& ops = output.script.operations;

    ops.push_back({chain::opcode::raw_data, to_chunk("witness")});
    for (const auto& witness : witness_list_) {
        ops.push_back({chain::opcode::special, witness});
    }

    ops.push_back({chain::opcode::raw_data, to_chunk("candidate")});
    for (const auto& candidate : candidate_list_) {
        ops.push_back({chain::opcode::special, candidate});
    }

    return output;
}

// DPOS_TODO: add algorithm to generate the witness list of each epoch
bool witness::calc_witness_list(uint64_t height)
{
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
    if (ops.size() < 2 + witess_number) {
        return false;
    }
    return true;
}

chain::block::ptr witness::fetch_vote_result_block(uint64_t height)
{
    auto vote_height = get_vote_result_height(height);

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
    chain::block::ptr sp_block;
    static std::once_flag s_flag;
    std::call_once(s_flag, [this, &sp_block, height](){sp_block = fetch_vote_result_block(height);});
    if (!sp_block) {
        return false;
    }
    return update_witness_list(*sp_block);
}

bool witness::update_witness_list(const chain::block& block)
{
    unique_lock ulock(mutex_);

    witness_list_.clear();
    candidate_list_.clear();

    const auto& transactions = block.transactions;
    auto& ops = transactions.front().outputs.back().script.operations;
    BITCOIN_ASSERT_MSG(ops.size() >= 2 + witess_number, "wrong number of vote result.");

    size_t i = 0;
    for (++i; i <= witess_number; ++i) {
        witness_list_.emplace_back(chain::operation::factory_from_data(ops[i].to_data()).data);
    }
    for (++i; i < ops.size(); ++i) {
        candidate_list_.emplace_back(chain::operation::factory_from_data(ops[i].to_data()).data);
    }
    return true;
}

uint32_t witness::get_slot_num(const witness_id& id) const
{
    shared_lock lock(mutex_);
    auto pos = finds(witness_list_, id);
    if (pos != witness_list_.end()) {
        return pos - witness_list_.cbegin();
    }
    return max_uint32;
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
    if (public_key.empty()) {
        return false;
    }

    if (out.back() != (h.number & 0xff)) {
        return false;
    }

    auto distinguished = out;
    distinguished.pop_back();

    ec_signature signature;

    if (!parse_signature(signature, distinguished, true)) {
        return false;
    }

    const auto sighash = h.hash();

    return bc::verify_signature(public_key, sighash, signature);
}

bool witness::verify_signer(const public_key_t& public_key, const chain::block& block, const chain::header& prev_header) const
{
    shared_lock lock(mutex_);
    auto witness_slot_num = get_slot_num(public_key);
    return verify_signer(witness_slot_num, block, prev_header);
}

bool witness::verify_signer(uint32_t witness_slot_num, const chain::block& block, const chain::header& prev_header) const
{
    if (witness_slot_num >= witess_number) {
        return false;
    }

    const auto& header = block.header;
    auto block_height = header.number;
    auto calced_slot_num = ((block_height - witness_enable_height) % witess_number);
    if (calced_slot_num == witness_slot_num) {
        return true;
    }
    if (((calced_slot_num + 1) % witess_number) == witness_slot_num) {
        // for safty, the previous slot should not be mined by the next witnesses
        return false;
    }
    static uint32_t time_config{24}; // same as HeaderAux::calculateDifficulty
    if (header.timestamp > prev_header.timestamp + time_config*2) {
        return true;
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
    return height - get_height_in_epoch(height);
}

bool witness::is_begin_of_epoch(uint64_t height)
{
    return is_witness_enabled(height) && get_height_in_epoch(height) == 0;
}

bool witness::is_between_vote_maturity_interval(uint64_t height)
{
    return is_witness_enabled(height) && get_height_in_epoch(height) <= vote_maturity;
}

bool witness::is_update_witness_needed(uint64_t height)
{
    return is_witness_enabled(height) && get_height_in_epoch(height) == vote_maturity;
}

} // consensus
} // libbitcoin
