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

uint32_t witness::pow_check_point_height = 100;
uint64_t witness::witness_enable_height = 2000000;
uint32_t witness::witness_number = 23;
uint32_t witness::epoch_cycle_height = 20000;
uint32_t witness::register_witness_lock_height = 10000;
uint64_t witness::witness_lock_threshold = 1000*(1e8); // ETP bits
uint32_t witness::vote_maturity = 24;

const uint32_t witness::max_candidate_count = 10000;
const uint32_t witness::witness_register_fee = 123456789;
const std::string witness::witness_registry_did = "witness_registry";

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

    if (instance_->setting_.use_testnet_rules) {
        witness::pow_check_point_height = 100;
        witness::witness_enable_height = 1000000;
        witness::witness_number = 5;
        witness::epoch_cycle_height = 1000;
        witness::register_witness_lock_height = 500;
        witness::witness_lock_threshold = 10*(1e8); // ETP bits
        witness::vote_maturity = 6;
    }

#ifdef PRIVATE_CHAIN
    witness::pow_check_point_height = 100;
    witness::witness_enable_height = 100;
    witness::witness_number = 23;
    witness::epoch_cycle_height = 100;
    witness::register_witness_lock_height = 50;
    witness::witness_lock_threshold = 1*(1e8); // ETP bits
    witness::vote_maturity = 6;
#endif

    BITCOIN_ASSERT(max_candidate_count >= witness_number);
    BITCOIN_ASSERT(epoch_cycle_height >= vote_maturity);
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
    for (const auto& witness : witness_list) {
        res += "\t" + witness_to_string(witness) + "\n";
    }
    res += "] ";
    return res;
}

size_t witness::get_witness_number()
{
    shared_lock lock(mutex_);
    return witness_list_.size();
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

    return output;
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
        return true;
    }

    auto stakeholders = chain.get_register_witnesses_with_stake(
        did_detail->get_address(), "", 0, height + register_witness_lock_height);
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
        if (!get_header(header, height-1)) {
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

#ifdef PRIVATE_CHAIN
    log::info(LOG_HEADER)
#else
    log::debug(LOG_HEADER)
#endif
        << "calc_witness_list at height " << height << ", " << show_list(witness_list);
    return true;
}

bool witness::is_vote_result_output(const chain::output& output)
{
    if (!output.is_etp() || output.value != 0) {
        return false;
    }

    auto& ops = output.script.operations;

    // check operation of script where mixhash is stored.
    if ((ops.size() != 1) || !chain::operation::is_push_only(ops)) {
        return false;
    }
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

bool witness::verify_vote_result(const chain::block& block, list& witness_list) const
{
    const auto& transactions = block.transactions;

    // check size of transactions
    if (transactions.size() != 1) {
        return false;
    }

    auto& coinbase_tx = transactions.front();

    // check size of outputs
    if (coinbase_tx.outputs.size() != 2) {
        log::debug(LOG_HEADER)
            << "in verify_vote_result -> no extra output to store witness mixhash, height " << block.header.number;
        return false;
    }

    auto& vote_result_output = coinbase_tx.outputs.back();

    // check operation of script where mixhash is stored.
    if (!is_vote_result_output(vote_result_output)) {
        return false;
    }

    if (!calc_witness_list(witness_list, block.header.number)) {
        log::debug(LOG_HEADER)
            << "in verify_vote_result -> calc_witness_list failed, height " << block.header.number;
        return false;
    }

    auto& ops = vote_result_output.script.operations;
    auto buff = chain::operation::factory_from_data(ops[0].to_data()).data;
    auto stored_mixhash = (h256::Arith)(h256((const uint8_t*)&buff[0], h256::ConstructFromPointer));
    auto calced_mixhash = (h256)calc_mixhash(witness_list);
    if (calced_mixhash != stored_mixhash) {
        log::debug(LOG_HEADER)
            << "in verify_vote_result -> verify mixhash failed, height " << block.header.number
            << ", stored_mixhash = " << stored_mixhash
            << ", calced_mixhash = " << calced_mixhash;
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

bool witness::update_witness_list(uint64_t height)
{
    if (!is_witness_enabled(height)) {
        list empty;
        swap_witness_list(empty);
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
    list witness_list;
    if (!verify_vote_result(block, witness_list)) {
#ifdef PRIVATE_CHAIN
        log::info(LOG_HEADER)
            << "verify vote result failed at height " << block.header.number;
#endif
        return false;
    }

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
        if (!consensus::witness::get().update_witness_list(height)) {
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

uint32_t witness::calc_slot_num(uint64_t block_height) const
{
    auto size = witness::get().get_witness_number();
    if (!is_witness_enabled(block_height) || size == 0) {
        return max_uint32;
    }

    auto calced_slot_num = ((block_height - witness_enable_height) % size);
    auto round_begin_height = get_round_begin_height(block_height);
    if (is_begin_of_epoch(round_begin_height)) {
        return calced_slot_num;
    }

    // remember latest calced offset to reuse it
    static std::pair<uint64_t, uint32_t> height_offset = {0, 0};

    uint32_t offset = 0;
    if (round_begin_height == height_offset.first) {
        offset = height_offset.second;
    }
    else {
        chain::header header;
        for (auto i = round_begin_height - size; i < round_begin_height; ++i) {
            if (!get_header(header, i)) {
                return max_uint32;
            }
            offset ^= hash_digest_to_uint(header.hash());
        }

        offset %= size;
        height_offset = std::make_pair(round_begin_height, offset);
    }

    return (calced_slot_num + offset) % size;
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

std::string witness::to_string(const public_key_t& public_key)
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
                               << to_string(public_key);
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
            << ", public_key is " << to_string(public_key)
            << ", signature is " << endorse_to_string(out)
            << ", hash is " << encode_hash(sighash);
#endif
        return false;
    }

    return true;
}

bool witness::verify_signer(const public_key_t& public_key, uint64_t height) const
{
    auto witness_slot_num = get_slot_num(to_witness_id(public_key));
    return verify_signer(witness_slot_num, height);
}

bool witness::verify_signer(uint32_t witness_slot_num, uint64_t height) const
{
    auto size = witness::get().get_witness_number();
    if (witness_slot_num >= size) {
        return false;
    }

    auto calced_slot_num = calc_slot_num(height);
    if (calced_slot_num == witness_slot_num) {
        return true;
    }

    return false;
}

bool witness::is_dpos_enabled()
{
    return false;
}

bool witness::is_witness_enabled(uint64_t height)
{
    return is_dpos_enabled() && height >= witness_enable_height;
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
    auto r = std::minmax(height1, height2);
    auto vote1 = get_vote_result_height(r.first);
    auto vote2 = get_vote_result_height(r.second);

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
    auto size = witness::get().get_witness_number();
    return is_witness_enabled(height) && size > 0
        ? height - ((height - witness_enable_height) % size)
        : 0;
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
