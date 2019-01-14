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
#include <metaverse/consensus/fts.hpp>
#include <metaverse/macros_define.hpp>
#include <metaverse/node/p2p_node.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <future>

#define LOG_HEADER "Witness"

namespace libbitcoin {
namespace consensus {

uint64_t witness::witness_enable_height = 2000000;
uint64_t witness::witness_register_enable_height = 2000000;
uint32_t witness::witness_number = 23;
uint32_t witness::epoch_cycle_height = 10000;
uint32_t witness::register_witness_lock_height = 20000; // it should larger than epoch_cycle_height
uint64_t witness::witness_lock_threshold = coin_price(1000); // ETP bits
uint32_t witness::vote_maturity = 120;

const uint32_t witness::max_candidate_count = 1024;
const uint32_t witness::witness_register_fee = 123456789;
const std::string witness::witness_registry_did = "witness_registry";

constexpr uint32_t dpos_missing_interval = 4;

witness* witness::instance_ = nullptr;

uint32_t hash_digest_to_uint(const hash_digest& hash)
{
    uint32_t result = 0;
    for (size_t i = 0; i < hash_size; i += 4) {
        result ^= ((hash[i]<<24) + (hash[i+1]<<16) + (hash[i+2]<<8) + hash[i+3]);
    }
    return result;
};

witness::witness(p2p_node& node)
    : node_(node)
    , setting_(node_.chain_impl().chain_settings())
    , witness_list_()
    , validate_block_(nullptr)
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

    if (instance_->is_testnet()) {
        witness::witness_enable_height = 1000000;
        witness::witness_register_enable_height = witness::witness_enable_height;
        witness::witness_number = 23;
        witness::epoch_cycle_height = 2300;
        witness::register_witness_lock_height = 5000;
        witness::witness_lock_threshold = coin_price(1000); // ETP bits
        witness::vote_maturity = 23;
    }

#ifdef PRIVATE_CHAIN
    witness::witness_enable_height = 5000;
    witness::witness_register_enable_height = 1000;
    witness::witness_number = 23;
    witness::epoch_cycle_height = 100;
    witness::register_witness_lock_height = 500;
    witness::witness_lock_threshold = coin_price(1000); // ETP bits
    witness::vote_maturity = 6;
#endif

    BITCOIN_ASSERT(max_candidate_count >= witness_number);
    BITCOIN_ASSERT(epoch_cycle_height >= vote_maturity);
    BITCOIN_ASSERT(register_witness_lock_height >= epoch_cycle_height);
}

witness& witness::create(p2p_node& node)
{
    static std::once_flag flag;
    std::call_once(flag, &witness::init, std::ref(node));
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
    res += "witness : [\n";
    for (auto& witness : witness_list) {
        res += "\t" + witness_to_address(witness) + "\n";
    }
    res += "] ";
    return res;
}

size_t witness::get_witness_number() const
{
    shared_lock lock(mutex_);
    return witness_list_.size();
}

bool witness::add_witness_vote_result(chain::transaction& coinbase_tx, uint64_t block_height)
{
    BITCOIN_ASSERT(coinbase_tx.is_coinbase());
    if (witness::is_begin_of_epoch(block_height)) {
        auto&& vote_output = create_witness_vote_result(block_height);
        if (vote_output.script.operations.empty()) {
            log::error(LOG_HEADER) << "create_witness_vote_result failed";
            return false;
        }

        coinbase_tx.outputs.emplace_back(vote_output);
        log::debug(LOG_HEADER) << "create_witness_vote_result complete. " << vote_output.to_string(1);
    }

    return true;
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

    auto mixhash = calc_mixhash(witness_list_);
    data_chunk chunk = to_chunk(h256(mixhash).asBytes());
    ops.push_back({chain::opcode::special, chunk});

    for (const auto& witness : witness_list_) {
        ops.push_back({chain::opcode::special, witness_to_public_key(witness)});
    }

    return output;
}

bool witness::is_vote_result_output(const chain::output& output)
{
    if (!output.is_etp() || output.value != 0) {
        return false;
    }

    auto& ops = output.script.operations;

    // check operation of script where mixhash is stored.
    if ((ops.size() < 1) || !chain::operation::is_push_only(ops)) {
        return false;
    }

    return true;
}

std::shared_ptr<witness::list> witness::get_block_witnesses(uint64_t height) const
{
    auto block = get_epoch_begin_block(height);
    if (!block) {
        log::error(LOG_HEADER)
            << "failed to get witness result block at height " << height;
        return nullptr;
    }

    return get_block_witnesses(*block);
}

std::shared_ptr<witness::list> witness::get_block_witnesses(const chain::block& block)
{
    // coinbase is always the first tx.
    auto& coinbase_tx = block.transactions.front();

    // witness output is always the last output.
    auto& vote_result_output = coinbase_tx.outputs.back();

    // check operation of script where mixhash is stored.
    if (!is_vote_result_output(vote_result_output)) {
        return nullptr;
    }

    auto witnesses = std::make_shared<list>();
    auto& ops = vote_result_output.script.operations;
    for (size_t i = 1; i < ops.size(); ++i) {
        const auto& data = chain::operation::factory_from_data(ops[i].to_data()).data;
        if (!is_public_key(data)) {
            log::error(LOG_HEADER)
                << "in get_block_witnesses ops is not public key, height " << block.header.number;
            return nullptr;
        }

        auto witness = to_witness_id(data);
        witnesses->emplace_back(witness);
    }

    return witnesses;
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

bool witness::calc_witness_list(list& witness_list, uint64_t height)
{
    if (!is_begin_of_epoch(height)) {
        log::error(LOG_HEADER) << "calc witness list must at the begin of epoch, not " << height;
        return false;
    }

    witness_list.clear();

    auto& chain = const_cast<blockchain::block_chain_impl&>(node_.chain_impl());
    std::shared_ptr<std::vector<std::string>> inactive_addresses;

    // if it's not the first epoch, get inactive witnesses of previous epoch.
    if (height >= witness_enable_height + epoch_cycle_height) {
        // get previous height of epoch
        uint64_t prev_epoch_height = get_epoch_begin_height(height - 1);
        inactive_addresses = get_inactive_witnesses(prev_epoch_height);

        log::info(LOG_HEADER) << inactive_addresses->size() << " inactive witnesses at epoch " << prev_epoch_height;
        for (auto& address : *inactive_addresses) {
            log::info(LOG_HEADER) << " > witness address: " << address;
        }
    }

    auto stakeholders = chain.get_witnesses_mars(height, inactive_addresses);
    if (stakeholders == nullptr || stakeholders->empty()) {
        return true;
    }

    if (stakeholders->size() <= witness_number) {
        for (const auto& stake_holder : *stakeholders) {
            witness_list.emplace_back(to_chunk(stake_holder->address()));
        }
    }
    else {
        chain::header header;
        if (!get_header(header, height - 1)) {
            return false;
        }

        if (stakeholders->size() > max_candidate_count) {
            std::sort(stakeholders->begin(), stakeholders->end(),
                [](const fts_stake_holder::ptr& h1, const fts_stake_holder::ptr& h2){
                    return h1->stake() > h2->stake(); // descendly
                });

            stakeholders->resize(max_candidate_count);
        }

        // pick witness_number candidates as witness randomly by fts
        uint32_t seed = hash_digest_to_uint(header.hash());
        auto selected_holders = fts::select_by_fts(*stakeholders, seed, witness_number);
        for (const auto& stake_holder : *selected_holders) {
            witness_list.emplace_back(to_chunk(stake_holder->address()));
        }
    }

    log::info(LOG_HEADER)
        << "calculated " << witness_list.size() << " witnesses at height "
        << height << ": " << show_list(witness_list);
    return true;
}

u256 witness::calc_mixhash(const list& witness_list)
{
    RLPStream s;
    s << witness_list.size();
    for (const auto& witness : witness_list) {
        s << bitcoin_hash(witness);
    }
    auto mix_hash = sha3(s.out());
    return mix_hash;
}

std::string witness::get_miner_address(const chain::block& block)
{
    const auto& output = block.transactions[0].outputs[0];
    BITCOIN_ASSERT(chain::operation::is_pay_key_hash_pattern(output.script.operations));
    return output.get_script_address();
}

std::shared_ptr<std::vector<std::string>> witness::get_inactive_witnesses(uint64_t height)
{
    using namespace consensus;

    uint64_t epoch_height = witness::get_epoch_begin_height(height);
    if (epoch_height == 0) {
        log::error(LOG_HEADER)
            << "failed to get_epoch_begin_height for height " << height;
        return nullptr;
    }

    // get witnesses at this epoch
    auto witnesses = get_block_witnesses(epoch_height);
    if (!witnesses) {
        log::error(LOG_HEADER)
            << "failed to get_block_witnesses at height " << epoch_height;
        return nullptr;
    }

    auto inactives = std::make_shared<std::vector<std::string>>();
    if (witnesses->empty()) {
        return inactives;
    }

    log::info(LOG_HEADER) << "total " << witnesses->size() << " witnesses at epoch " << height;
    // map address and witness_id
    std::vector<std::string> addresses;
    for (auto& witness : *witnesses) {
        auto address = witness_to_address(witness);
        addresses.push_back(address);

        log::info(LOG_HEADER) << " > witness address: " << address;
    }

    // statistic votes
    std::map<std::string, uint32_t> votes;
    auto start = epoch_height + vote_maturity;
    auto end = epoch_height + epoch_cycle_height - vote_maturity;
    uint32_t total_vote = 0;
    for (auto h = start; h < end; ++h) {
        ec_compressed public_key(null_compressed_point);
        node_.chain_impl().fetch_block_public_key(h,
            [&public_key](const code& ec, const ec_compressed& pubkey) {
                if (ec) {
                    return;
                }
                public_key = pubkey;
            });

        if (!is_public_key(public_key)) {
            continue;
        }

        auto address = witness_to_address(to_chunk(encode_base16(public_key)));

        if (votes.find(address) != votes.end()) {
            votes[address] += 1;
        }
        else {
            votes[address] = 1;
        }

        ++total_vote;
    }

    // nobody votes.
    auto size = votes.size();
    if (size == 0) {
        *inactives = addresses;
        return inactives;
    }

    // find addresses that did not vote.
    for (auto& address : addresses) {
        if (votes.find(address) == votes.end()) {
            inactives->push_back(address);
        }
    }

    log::info(LOG_HEADER) << votes.size() << " active witnesses at epoch " << height;
    for (auto& entry : votes) {
        log::info(LOG_HEADER) << " > witness address: " << entry.first << ", pack " << entry.second << " blocks";
    }

    // find addresses that had low vote percentage.
    for (auto& entry : votes) {
        if (entry.second * 5 < total_vote) {
            inactives->push_back(entry.first);
        }
    }

    return inactives;
}

chain::block::ptr witness::fetch_block(uint64_t height) const
{
    if (validate_block_) {
        return validate_block_->fetch_full_block(height);
    }

    std::promise<code> p;
    chain::block::ptr sp_block;

    node_.chain_impl().fetch_block(height,
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
        list empty;
        swap_witness_list(empty);
        return true;
    }

    auto block = get_epoch_begin_block(height);
    if (!block) {
        return true;
    }

    return update_witness_list(*block);
}

bool witness::update_witness_list(const chain::block& block)
{
    uint64_t height = block.header.number;

    // get witnesses from block
    std::shared_ptr<list> stored_witnesses = get_block_witnesses(block);
    if (!stored_witnesses) {
        log::error(LOG_HEADER)
            << "update_witness_list: can not get witnesses at height " << height;
        return false;
    }

    // calculate witnesses on blockchain
    list witness_list;
    if (!calc_witness_list(witness_list, height)) {
        log::error(LOG_HEADER)
            << "update_witness_list: calc_witness_list failed, height " << height;
        return false;
    }

    // verify witnesses
    if (stored_witnesses->size() != witness_list.size()) {
        log::error(LOG_HEADER)
            << "update_witness_list: verify witness size failed, height " << height
            << ", stored size = " << stored_witnesses->size()
            << ", calced size = " << witness_list.size();
        return false;
    }

    auto stored_mixhash = (h256)calc_mixhash(*stored_witnesses);
    auto calced_mixhash = (h256)calc_mixhash(witness_list);
    if (calced_mixhash != stored_mixhash) {
        log::error(LOG_HEADER)
            << "update_witness_list: verify witness mixhash failed, height " << height
            << ", stored_mixhash = " << stored_mixhash
            << ", calced_mixhash = " << calced_mixhash;
        return false;
    }

    // swap witnesses
    swap_witness_list(witness_list);
    return true;
}

bool witness::init_witness_list()
{
    if (!is_dpos_enabled()) {
        return true;
    }

    auto& chain = node_.chain_impl();
    chain.set_sync_disabled(true);
    unique_lock lock(chain.get_mutex());

    uint64_t height = 0;
    if (!chain.get_last_height(height)) {
        log::info(LOG_HEADER) << "get last height failed";
        return false;
    }

    if (consensus::witness::is_witness_enabled(height)) {
        if (!update_witness_list(height)) {
            log::info(LOG_HEADER) << "update witness list failed at height " << height;
            return false;
        }
        log::info(LOG_HEADER) << "update witness list succeed at height " << height;
    }

    chain.set_sync_disabled(false);
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

    return max_uint32;
}

/// @return (actual num, expect num) pair
/// If actual num != expect num, then there exist missed slot
std::pair<uint32_t, uint32_t> witness::calc_slot_num(uint64_t block_height) const
{
    auto size = get_witness_number();
    if (!is_witness_enabled(block_height) || size == 0) {
        return std::make_pair(max_uint32, max_uint32);
    }

    auto begin_height = get_epoch_begin_height(block_height);
    uint64_t first_height = begin_height + vote_maturity;
    uint32_t start_num = 0;

    auto prev_dpos_header = node_.chain_impl().get_prev_block_header(block_height, chain::block_version_dpos);
    if (prev_dpos_header && prev_dpos_header->number > begin_height) {
        first_height = prev_dpos_header->number + 1;
        start_num = (static_cast<uint32_t>(prev_dpos_header->nonce) + 1) % size;
    }

    auto calced_slot_num = start_num;
    if (block_height > first_height) {
        auto offset = (block_height - first_height) / dpos_missing_interval;
        calced_slot_num = (start_num + offset) % size;
    }

    return std::make_pair(calced_slot_num, start_num);
}

bool witness::is_testnet()
{
    return setting_.use_testnet_rules;
}

std::string witness::witness_to_address(const witness_id& witness)
{
    uint32_t payment_version = wallet::payment_address::mainnet_p2kh;
    if (witness::get().is_testnet()) {
        payment_version = 127;  // testnet
    }

    wallet::ec_public ep = wallet::ec_public(witness_to_string(witness));
    wallet::payment_address pay_address(ep, payment_version);
    return pay_address.encoded();
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

witness::witness_id witness::to_witness_id(const data_slice& public_key)
{
    return to_chunk(encode_base16(public_key));
}

std::string witness::to_string(const data_slice& public_key)
{
    return encode_base16(public_key);
}

bool witness::verify_signer(const data_slice& public_key, uint64_t height) const
{
    auto witness_slot_num = get_slot_num(to_witness_id(public_key));
    return verify_signer(witness_slot_num, height);
}

bool witness::verify_signer(uint32_t witness_slot_num, uint64_t height) const
{
    auto size = get_witness_number();
    if (witness_slot_num >= size) {
        return false;
    }

    auto result_pair = calc_slot_num(height);
    auto calced_slot_num = result_pair.first;
    if (calced_slot_num != witness_slot_num) {
        return false;
    }

    auto start_num = result_pair.second;
    std::vector<std::string> missed_vec;
    for (auto num = start_num; num != calced_slot_num; num = (num+1)%size) {
        missed_vec.push_back(std::to_string(num));
    }
    auto missed_nums = missed_vec.empty() ? std::string("none") : bc::join(missed_vec);

    log::info(LOG_HEADER) << "calc_slot_num at " << height
        << ", start num: " << start_num
        << ", slot num: " << calced_slot_num
        << ", missed num: " << missed_nums;

    return true;
}

bool witness::is_dpos_enabled()
{
#ifdef PRIVATE_CHAIN
    return true;
#endif

    if (instance_->is_testnet()) {
        return true;
    }
    else {
        return false;
    }
}

bool witness::is_witness_enabled(uint64_t height)
{
    return is_dpos_enabled() && height >= witness_enable_height;
}

uint64_t witness::get_height_in_epoch(uint64_t height)
{
    return (height - witness_enable_height) % epoch_cycle_height;
}

chain::block::ptr witness::get_epoch_begin_block(uint64_t height) const
{
    auto vote_height = get_epoch_begin_height(height);
    if (vote_height == 0) {
        return nullptr;
    }

    return fetch_block(vote_height);
}

// vote result is stored in the beginning of each epoch
uint64_t witness::get_epoch_begin_height(uint64_t height)
{
    return is_witness_enabled(height) ? (height - get_height_in_epoch(height)) : 0;
}

bool witness::is_begin_of_epoch(uint64_t height)
{
    return is_witness_enabled(height) && get_height_in_epoch(height) == 0;
}

bool witness::is_in_same_epoch(uint64_t height1, uint64_t height2)
{
    auto r = std::minmax(height1, height2);
    auto vote1 = get_epoch_begin_height(r.first);
    auto vote2 = get_epoch_begin_height(r.second);

    if (vote1 == vote2) {
        return true;
    }

    if (vote2 == vote1 + epoch_cycle_height) {
        return vote2 - r.first < vote_maturity;
    }

    return false;
}

uint64_t witness::get_round_begin_height(uint64_t height)
{
    if (!is_witness_enabled(height)) {
        return 0;
    }

    auto size = witness::get().get_witness_number();
    if (size == 0) {
        return 0;
    }

    return (height - get_epoch_begin_height(height)) % size;
}

bool witness::get_header(chain::header& out_header, uint64_t height) const
{
    if (validate_block_) {
        return validate_block_->get_header(out_header, height);
    }
    return node_.chain_impl().get_header(out_header, height);
}

void witness::set_validate_block(const validate_block* validate_block)
{
    validate_block_ = validate_block;
}

witness_with_validate_block_context::witness_with_validate_block_context(
    witness& w, const validate_block* v)
    : witness_(w)
{
    witness_.set_validate_block(v);
}

witness_with_validate_block_context::~witness_with_validate_block_context()
{
    witness_.set_validate_block(nullptr);
}

} // consensus
} // libbitcoin
