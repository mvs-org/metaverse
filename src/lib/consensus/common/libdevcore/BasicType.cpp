#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/macros_define.hpp>
#include <metaverse/bitcoin/utility/random.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>
#include "crypto/common.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>

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
    RLPStream s;
    s << (bigint) bi.version << (bigint)bi.number
        << bi.previous_block_hash << (bigint) bi.timestamp
        << stake.point.hash << (bigint) stake.point.index;

    auto hash_pos = bitcoin_hash(to_chunk(s.out()));
    h256 memo = h256(encode_hash(hash_pos));
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
    const chain::header::ptr pprev)
{
#ifdef ENABLE_PILLAR
    if (current.number < pos_enabled_height) {
        return calculate_difficulty_v1(current, prev, pprev);
    }

    return calculate_difficulty_v2(current, prev, pprev);
#else
    return calculate_difficulty_v1(current, prev, pprev);
#endif
}

// Do not modify this function! It's for backward compatibility.
u256 HeaderAux::calculate_difficulty_v1(
    const chain::header& current,
    const chain::header::ptr prev,
    const chain::header::ptr pprev)
{
    if (!current.number) {
        throw GenesisBlockCannotBeCalculated();
    }

    auto minimumDifficulty = is_testnet ? bigint(300000) : bigint(914572800);
    bigint target(minimumDifficulty);

    // DO NOT MODIFY time_config in release
    static uint32_t time_config{24};
    if (prev) {
        if (current.timestamp >= prev->timestamp + time_config) {
            target = prev->bits - (prev->bits/1024);
        }
        else {
            target = prev->bits + (prev->bits/1024);
        }
    }

    bigint result = std::max<bigint>(minimumDifficulty, target);

#ifdef PRIVATE_CHAIN
    result = bigint(10);
#endif

    return u256(std::min<bigint>(result, std::numeric_limits<u256>::max()));
}

uint32_t HeaderAux::limit_timespan(uint32_t timespan)
{
    // Limit adjustment step
    if (timespan < block_timespan_window / 10) {
        return block_timespan_window / 10;
    }

    if (timespan > block_timespan_window * 2) {
        return uint32_t(block_timespan_window * 1.5);
    }

    return timespan;
}

bigint HeaderAux::adjust_difficulty(uint32_t timespan, bigint & result)
{
    // Limit adjustment step
    timespan = limit_timespan(timespan);

    // Retarget
    const uint32_t interval = 40;
    result *= ((interval + 1) * block_timespan_window);
    result /= ((interval - 1) * block_timespan_window + timespan * 2);
    return result;
}

u256 HeaderAux::calculate_difficulty_v2(
    const chain::header& current,
    const chain::header::ptr prev,
    const chain::header::ptr pprev)
{
    auto minimumDifficulty = is_testnet ? bigint(300000) : bigint(1024 * 1024);
    bool isPoW = (current.version == chain::block_version_pow);

#ifdef PRIVATE_CHAIN
    if (isPoW) {
        minimumDifficulty = bigint(300000);
    }
#endif

    if (nullptr == prev || nullptr == pprev) {
        return u256(minimumDifficulty);
    }

    BITCOIN_ASSERT(current.version == prev->version);
    BITCOIN_ASSERT(current.version == pprev->version);

    bigint prev_bits = prev->bits;
    uint32_t prev_timespan = prev->timestamp - pprev->timestamp;
    uint32_t curr_timespan = current.timestamp - prev->timestamp;
    uint32_t timespan = isPoW ? curr_timespan : prev_timespan;

    // Retarget
    prev_bits = adjust_difficulty(timespan, prev_bits);

    auto result = std::max<bigint>(prev_bits, minimumDifficulty);
    result = std::min<bigint>(result, std::numeric_limits<u256>::max());

#ifdef PRIVATE_CHAIN
    if (isPoW) {
        if (current.number < 6000) {
            result = bigint(10);
        }
    }
#endif

#ifdef ENABLE_PILLAR
    if (current.transaction_count > 0) {
        log::info("difficulty")
            << "last " << chain::get_block_version(current)
            << " timespan: " << timespan << " s, current height: "
            << current.number << ", bits: " << result;
    }
#endif

    return u256(result);
}
