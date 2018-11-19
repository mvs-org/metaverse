#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/bitcoin/math/uint256.hpp>
#include <metaverse/bitcoin/utility/random.hpp>
#include "crypto/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>

using namespace libbitcoin;

DEV_SIMPLE_EXCEPTION(GenesisBlockCannotBeCalculated);
/*****************************/
WorkPackage::WorkPackage(chain::header& _bh):
    boundary(HeaderAux::boundary(_bh)),
    headerHash(HeaderAux::hashHead(_bh)),
    seedHash(HeaderAux::seedHash(_bh))
{}
/****************************/

LightAllocation::LightAllocation(h256& _seedHash)
{
    uint64_t blockNumber = HeaderAux::number(_seedHash);
    light = ethash_light_new(blockNumber);
    if (!light)
        BOOST_THROW_EXCEPTION(ExternalFunctionFailure("ethash_light_new()"));
    size = ethash_get_cachesize(blockNumber);
}

LightAllocation::~LightAllocation()
{
    ethash_light_delete(light);
}

Result LightAllocation::compute(h256& _headerHash, Nonce& _nonce)
{
    ethash_return_value r = ethash_light_compute(light, *(ethash_h256_t*)_headerHash.data(), (uint64_t)(u64)_nonce);
    if (!r.success)
        BOOST_THROW_EXCEPTION(ExternalFunctionFailure("DAGCreationFailure"));
    return Result{h256((uint8_t*)&r.result, h256::ConstructFromPointer), h256((uint8_t*)&r.mix_hash, h256::ConstructFromPointer)};
}

/*****************************/
FullAllocation::FullAllocation(ethash_light_t _light, ethash_callback_t _cb)
{
    full = ethash_full_new(_light, _cb);
    if (!full)
    {
        BOOST_THROW_EXCEPTION(ExternalFunctionFailure("ethash_full_new"));
    }
}

Result FullAllocation::compute(h256& _headerHash, Nonce& _nonce)
{
    ethash_return_value_t r = ethash_full_compute(full, *(ethash_h256_t*)_headerHash.data(), (uint64_t)(u64)_nonce);
    if (!r.success)
        BOOST_THROW_EXCEPTION(ExternalFunctionFailure("DAGCreationFailure"));
    return Result{h256((uint8_t*)&r.result, h256::ConstructFromPointer), h256((uint8_t*)&r.mix_hash, h256::ConstructFromPointer)};
}


FullAllocation::~FullAllocation()
{
    ethash_full_delete(full);
}
/*****************************/

HeaderAux* HeaderAux::s_this = nullptr;
bool HeaderAux::is_testnet = false;


HeaderAux* HeaderAux::get()
{
    static std::once_flag flag;
    std::call_once(flag, []{s_this = new HeaderAux();});
    return s_this;
}


h256 HeaderAux::seedHash(const chain::header& bi)
{
    unsigned _number = (unsigned) bi.number;
    unsigned epoch = _number / ETHASH_EPOCH_LENGTH;
    Guard l(get()->x_epochs);

    if (epoch >= get()->m_seedHashes.size()) {
        h256 ret;
        unsigned n = 0;
        if (!get()->m_seedHashes.empty()) {
            ret = get()->m_seedHashes.back();
            n = get()->m_seedHashes.size() - 1;
        }

        get()->m_seedHashes.resize(epoch + 1);
//        cdebug << "Searching for seedHash of epoch " << epoch;

        for (; n <= epoch; ++n, ret = sha3(ret)) {
            get()->m_seedHashes[n] = ret;
//            cdebug << "Epoch" << n << "is" << ret;
        }
    }
    return get()->m_seedHashes[epoch];
}

uint64_t HeaderAux::number(h256& _seedHash)
{
    Guard l(get()->x_epochs);
    unsigned epoch = 0;
    auto epochIter = get()->m_epochs.find(_seedHash);
    if (epochIter == get()->m_epochs.end()) {
        //        cdebug << "Searching for seedHash " << _seedHash;
        for (h256 h; h != _seedHash && epoch < 2048; ++epoch, h = sha3(h), get()->m_epochs[h] = epoch)
        {}

        if (epoch == 2048) {
            std::ostringstream error;
            error << "apparent block number for " << _seedHash << " is too high; max is " << (ETHASH_EPOCH_LENGTH * 2048);
            throw std::invalid_argument(error.str());
        }
    }
    else
        epoch = epochIter->second;
    return epoch * ETHASH_EPOCH_LENGTH;
}

h256 HeaderAux::boundary(const chain::header& bi)
{
    auto d = bi.bits;
    return d ? (h256)u256(((bigint(1) << 255)-bigint(1) +(bigint(1) << 255) ) / d) : h256();
}

h256 HeaderAux::hashHead(const chain::header& bi)
{
    h256 memo;
    RLPStream s;
    s  << (bigint) bi.version << (bigint)bi.bits << (bigint)bi.number << bi.merkle
        << bi.previous_block_hash << (bigint) bi.timestamp ;
    memo = sha3(s.out());
    return memo;
}

h256 HeaderAux::hash_head_pos(const chain::header& bi, const chain::output_info& stake)
{
    h256 memo;
    RLPStream s;
    s  << (bigint) bi.timestamp << (bigint)stake.data.value << stake.point.hash << (bigint) stake.point.index;
    memo = sha3(s.out());
    return memo;
}

uint64_t HeaderAux::cacheSize(const chain::header& _header)
{
    return ethash_get_cachesize((uint64_t)_header.number);
}

uint64_t HeaderAux::dataSize(uint64_t _blockNumber)
{
    return ethash_get_datasize(_blockNumber);
}

u256 HeaderAux::calculate_difficulty(
    const chain::header& current,
    const chain::header::ptr prev,
    bool is_staking)
{
    if (is_staking) {
        return calculate_difficulty_pos(current, prev);
    }
    else {
        return calculate_difficulty_pow(current, prev);
    }
}

u256 HeaderAux::calculate_difficulty_pow(const chain::header& current, const chain::header::ptr prev)
{
    if (!current.number) {
        throw GenesisBlockCannotBeCalculated();
    }

    /// test-private-chain
    auto minimumDifficulty = bigint(10);
    // auto minimumDifficulty = is_testnet ? bigint(300000) : bigint(914572800);
    bigint target(minimumDifficulty);

    if (nullptr != prev) {
        // DO NOT MODIFY time_config in release
        static uint32_t time_config{24};
        if (current.timestamp >= prev->timestamp + time_config) {
            target = prev->bits - (prev->bits/1024);
        }
        else {
            target = prev->bits + (prev->bits/1024);
        }
    }

    bigint result = std::max<bigint>(minimumDifficulty, target);
    return u256(std::min<bigint>(result, std::numeric_limits<u256>::max()));
}

h256 uint_to_hash256(const uint256_t &a) {
    h256 b;
    auto& array = b.asArray();
    for (int x = 0; x < a.size(); ++x) {
        WriteLE32(array.begin() + x*4, *(a.begin() + x));
    }
    return b;
}

uint256_t hash_to_uint56(const h256 &a)
{
    uint256_t b;
    auto& array = a.asArray();
    for (int x = 0; x < b.size(); ++x) {
        *(b.begin() + x) = ReadLE32(array.begin() + x*4);
    }
    return b;
}

u256 HeaderAux::calculate_difficulty_pos(const chain::header& current, const chain::header::ptr prev)
{
    h256 pos_limit_target = h256("000000000000ffffffffffffffffffffffffffffffffffffffffffffffffffff");
    uint256_t nbits_limit_pos = hash_to_uint56(pos_limit_target);
    uint32_t pos_target_timespan = 24;

    uint32_t limit_target = nbits_limit_pos.GetCompact();
    // if (nullptr == prev) {
    //     return limit_target;
    // }

    // uint32_t last_pos_bit = (uint32_t)prev->bits;
    // uint32_t last_pos_time = prev->timestamp;

    uint32_t last_pos_bit = limit_target;
    uint32_t last_pos_time = current.timestamp;
    const auto random = pseudo_random(1, pos_target_timespan * 2);
    const auto offset = static_cast<uint32_t>(random);
    // TODO get from prev prev block
    uint32_t llast_pos_time = last_pos_time - offset;

    // Limit adjustment step
    uint32_t actual_timespan = last_pos_time - llast_pos_time;
    if (actual_timespan < pos_target_timespan / 4) {
        actual_timespan = pos_target_timespan / 4;
    }
    if (actual_timespan > pos_target_timespan * 4) {
        actual_timespan = pos_target_timespan * 4;
    }

    // Retarget
    uint256_t new_target;
    new_target.SetCompact(last_pos_bit);
    new_target /= pos_target_timespan;
    new_target *= actual_timespan;

    log::info("calculate_difficulty") << "pos limit target: " << pos_limit_target;
    log::info("calculate_difficulty") << "    limit target: " << uint_to_hash256(nbits_limit_pos);
    log::info("calculate_difficulty") << "     prev target: " << uint_to_hash256(new_target);
    if (new_target > nbits_limit_pos) {
        new_target = nbits_limit_pos;
    }

    uint32_t value = new_target.GetCompact();
    log::info("calculate_difficulty")
        << "nbits_limit_pos: " << std::to_string(limit_target)
        << "      last bits: " << std::to_string(last_pos_bit)
        << "   current bits: " << std::to_string(value);

    return u256(value);
}
