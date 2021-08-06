
#include <metaverse/consensus/miner/MinerAux.h>
#include <chrono>
#include <array>
#include <thread>
#include <random>
#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>
#include <metaverse/macros_define.hpp>
#include <metaverse/consensus/libethash/internal.h>
#include <metaverse/consensus/libdevcore/Guards.h>
#include <metaverse/consensus/libdevcore/Log.h>
#include <metaverse/consensus/libdevcore/SHA3.h>
#include <metaverse/consensus/libdevcore/Exceptions.h>
#include <metaverse/bitcoin/math/uint256.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/utility/log.hpp>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>

#if BOOST_VERSION < 107300
#include <boost/detail/endian.hpp>
#else
#include "boost_endian_compat.h"
#include <boost/predef/other/endian.h>
#endif

using namespace libbitcoin;
using namespace std;

MinerAux* libbitcoin::MinerAux::s_this = nullptr;
#define LOG_MINER "etp_hash"

MinerAux::~MinerAux()
{
}

MinerAux* MinerAux::get()
{
    static std::once_flag flag;
    std::call_once(flag, []{s_this = new MinerAux();});
    return s_this;
}

LightType MinerAux::get_light(h256& _seedHash)
{
    UpgradableGuard l(get()->x_lights);
    if (get()->m_lights.count(_seedHash))
        return get()->m_lights.at(_seedHash);
    UpgradeGuard l2(l);
    return (get()->m_lights[_seedHash] = make_shared<LightAllocation>(_seedHash));
}

//static std::function<int(unsigned)> s_dagCallback;

static int dagCallbackShim(unsigned _p)
{
    return 0;
}

FullType MinerAux::get_full(h256& _seedHash)
{
    FullType ret;
    auto l = get_light(_seedHash);
    DEV_GUARDED(get()->x_fulls)
    if ((ret = get()->m_fulls[_seedHash].lock()))
    {
        get()->m_lastUsedFull = ret;
        return ret;
    }
    //s_dagCallback = _f;
    ret = make_shared<FullAllocation>(l->light, dagCallbackShim);
    DEV_GUARDED(get()->x_fulls)
    get()->m_fulls[_seedHash] = get()->m_lastUsedFull = ret;
    return ret;
}

bool MinerAux::search(libbitcoin::chain::header& header, std::function<bool (void)> is_exit)
{
    auto tid = std::this_thread::get_id();
    static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));
    uint64_t tryNonce = s_eng();
    ethash_return_value ethashReturn;
    FullType dag;
    h256 seed = HeaderAux::seedHash(header);
    h256 header_hash = HeaderAux::hashHead(header);
    h256 boundary = HeaderAux::boundary(header);
    std::chrono::steady_clock::time_point timeStart;
    uint64_t ms;
    uint64_t hashCount = 1;

    // quick check and exit
    if (is_exit() == true) {
        return false;
    }

    while( nullptr == dag) {
        log::debug(LOG_MINER) << "start generate dag\n";
        try {
            dag = get_full(seed);
        } catch (const ExternalFunctionFailure&) {
            return false;
        }
    }

    log::debug(LOG_MINER) << "Start miner @ height:  "<< header.number << '\n';

    timeStart = std::chrono::steady_clock::now();
    for (; ; tryNonce++, hashCount++) {
        ethashReturn = ethash_full_compute(dag->full, *(ethash_h256_t*)header_hash.data(), tryNonce);
        h256 value = h256((uint8_t*)&ethashReturn.result, h256::ConstructFromPointer);
        h256 mixhash =h256((uint8_t*)&ethashReturn.mix_hash, h256::ConstructFromPointer);
        if (value <= boundary ) {
            MinerAux::setNonce(header, (u64)tryNonce);
            MinerAux::setMixHash(header, mixhash);
            log::debug(LOG_MINER) << "find slolution! block height: "<< header.number << '\n';
            break;
        }

        if (is_exit() == true) {
            return false;
        }
    }

    ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeStart).count();
    ms = ms? ms : 1;
    get()->m_rate = hashCount * 1000 / ms;

    return ethashReturn.success;
}

bool MinerAux::verify_work(const libbitcoin::chain::header& header, const libbitcoin::chain::header::ptr parent)
{
    Result result;
    h256 seedHash = HeaderAux::seedHash(header);
    h256 headerHash  = HeaderAux::hashHead(header);
    Nonce nonce = (Nonce)header.nonce;

    DEV_GUARDED(get()->x_fulls)
    if (FullType dag = get()->m_fulls[seedHash].lock()) {
        result = dag->compute(headerHash, nonce);

        if (result.value <= HeaderAux::boundary(header)
            && (result.mixHash).hex() == ((h256)header.mixhash).hex()) {
            //log::debug(LOG_MINER) << header.number <<" block has been verified (Full)\n";
            return true;
        }
        return false;
    }

    result = get()->get_light(seedHash)->compute(headerHash, nonce);
    if (result.value <= HeaderAux::boundary(header)
        && (result.mixHash).hex() == ((h256)header.mixhash).hex()) {
        //log::debug(LOG_MINER) << header.number <<" block has been verified (Light)\n";
        return true;
    }

    log::error(LOG_MINER) << header.number << " block  verified failed !\n";
    return false;
}

bool MinerAux::verify_stake(const chain::header& header, const chain::output_info& stake_output)
{
    // Base target
    h256 boundary = HeaderAux::boundary(header);
    uint64_t amount = std::max<uint64_t>(1, (stake_output.data.value / coin_price()));
    uint64_t s = std::round(log2l(amount));
    uint64_t stake = (s * s * s * pos_stake_factor);

    // Calculate hash
    u256 pos = HeaderAux::hash_head_pos(header, stake_output);
    pos /= u256(stake);

    bool succeed = (h256(pos) <= boundary);

#ifdef MVS_DEBUG
    if (header.transaction_count > 0) {
        uint64_t coin_age = header.number - stake_output.height;
        log::info("verify_stake")
            << (succeed ? "True" : "False")
            << ", stake amount: " << (uint64_t)(stake_output.data.value / coin_price())
            << " ETPs, coin_age: " << coin_age
            << ", height: " << header.number
            << ", bits: " << header.bits
            << ", pos: " << h256(pos)
            << ", boundary: " << boundary;
    }
#endif

    return succeed;
}
