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
    if (current.number < pos_enabled_height) {
        return calculate_difficulty_v1(current, prev, pprev);
    }

    return calculate_difficulty_v2(current, prev, pprev);
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
    if (prev) {
        if (current.timestamp >= prev->timestamp + 24u) { // approach to 24*1.4 seconds
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

u256 HeaderAux::calculate_difficulty_v2(
    const chain::header& current,
    const chain::header::ptr prev,
    const chain::header::ptr pprev)
{
    bigint minimumDifficulty, result;
    int32_t interval;

    if (current.version == chain::block_version_pow) {
        //////////// PoW ///////////////
        minimumDifficulty = is_testnet ? bigint(300000) : bigint(914572800);
#ifdef PRIVATE_CHAIN
        minimumDifficulty = bigint(300000);
        if (current.number < 10000) {
            return u256(bigint(10));
        }
#endif

        if (nullptr == prev) {
            return u256(minimumDifficulty);
        }

        BITCOIN_ASSERT(current.version == prev->version);
        const bigint& parent_bits = prev->bits;
        interval = current.timestamp - prev->timestamp;
        const bigint adjustvalue = bigint(std::max<int32_t>(2 - interval/10, -99)); // approach to 20*1.4 seconds
        const bigint&& target = parent_bits + parent_bits / 2048 * adjustvalue;
        result = std::max<bigint>(target, minimumDifficulty);
    }
    else {
        //////////// PoS ///////////////
        minimumDifficulty = is_testnet ? bigint(1000000) : bigint(100000000);
        if (nullptr == prev || nullptr == pprev) {
            return u256(minimumDifficulty);
        }

        BITCOIN_ASSERT(current.version == prev->version);
        const bigint& parent_bits = prev->bits;
        interval = prev->timestamp - pprev->timestamp;;
        const bigint adjustvalue = bigint(std::max<int32_t>(18 - interval/10 ,-99)); // approach to 180*1.4 seconds
        const bigint&& target = parent_bits + parent_bits / 2048 * adjustvalue;
        result = std::max<bigint>(target, minimumDifficulty);
    }

    if (current.transaction_count > 0) {
        log::info("difficulty")
            << "{ \"block_type\" : \"" << chain::get_block_version(current) << "\""
            << ", \"last_interval\" : " << interval
            << ", \"current_height\" : " << current.number
            << ", \"difficulty\" : " << result << " }";
    }

    // Retarget
    return u256(std::min<bigint>(result, std::numeric_limits<u256>::max()));
}
