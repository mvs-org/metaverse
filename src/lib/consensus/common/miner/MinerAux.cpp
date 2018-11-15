#include <metaverse/consensus/libethash/internal.h>
#include <metaverse/consensus/libdevcore/Guards.h>
#include <metaverse/consensus/libdevcore/Log.h>
#include <metaverse/consensus/libdevcore/SHA3.h>
#include <metaverse/bitcoin/chain/header.hpp>
#include <boost/detail/endian.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <array>
#include <thread>
#include <metaverse/consensus/miner/MinerAux.h>
#include <random>
#include <metaverse/consensus/libdevcore/Exceptions.h>
#include <boost/throw_exception.hpp>
#include <metaverse/bitcoin/utility/log.hpp>


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

    while( nullptr == dag)
    {
        log::debug(LOG_MINER) << "start generate dag\n";
        try {
            dag = get_full(seed);
        } catch (const ExternalFunctionFailure&) {
            return false;
        }
    }
    log::debug(LOG_MINER) << "Start miner @ height:  "<< header.number << '\n';
    timeStart = std::chrono::steady_clock::now();
    for (; ; tryNonce++, hashCount++)
    {
        ethashReturn = ethash_full_compute(dag->full, *(ethash_h256_t*)header_hash.data(), tryNonce);
        h256 value = h256((uint8_t*)&ethashReturn.result, h256::ConstructFromPointer);
        h256 mixhash =h256((uint8_t*)&ethashReturn.mix_hash, h256::ConstructFromPointer);
        if (value <= boundary )
        {
            MinerAux::setNonce(header, (u64)tryNonce);
            MinerAux::setMixHash(header, mixhash);
            log::debug(LOG_MINER) << "find slolution! block height: "<< header.number << '\n';
            break;
        }
        if(is_exit() == true)
        {
            ethashReturn.success = false;
            return ethashReturn.success;
        }
    }
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timeStart).count();
    ms = ms? ms : 1;
    get()->m_rate = hashCount * 1000 / ms;

    return ethashReturn.success;
}

bool MinerAux::verifySeal(libbitcoin::chain::header& _header, libbitcoin::chain::header& _parent)
{
    Result result;
    h256 seedHash = HeaderAux::seedHash(_header);
    h256 headerHash  = HeaderAux::hashHead(_header);
    Nonce nonce = (Nonce)_header.nonce;
    if( _header.bits != HeaderAux::calculateDifficulty(_header, _parent))
    {
        log::error(LOG_MINER) << _header.number<<" block , verify diffculty failed\n";
        return false;
    }
    DEV_GUARDED(get()->x_fulls)
    if (FullType dag = get()->m_fulls[seedHash].lock())
    {
        result = dag->compute(headerHash, nonce);

        if(result.value <= HeaderAux::boundary(_header) && (result.mixHash).hex() == ((h256)_header.mixhash).hex())
        {
            //log::debug(LOG_MINER) << _header.number <<" block has been verified (Full)\n";
            return true;
        }
        return false;
    }
    result = get()->get_light(seedHash)->compute(headerHash, nonce);
    if(result.value <= HeaderAux::boundary(_header) && (result.mixHash).hex() == ((h256)_header.mixhash).hex())
    {
        //log::debug(LOG_MINER) << _header.number <<" block has been verified (Light)\n";
        return true;
    }
    log::error(LOG_MINER) << _header.number <<" block  verified failed !\n";
    return false;
}


bool MinerAux::check_kernel(chain::header& header, const chain::output_info& info, uint64_t height)
{
    return MinerAux::check_proof_of_stake(header, info, height);
}

bool MinerAux::check_proof_of_stake(chain::header& header, const chain::output_info& info, uint64_t height)
{
    u256& target = header.bits;
    h256 pos = HeaderAux::hash_head_pos(header, info, height);

    log::info(LOG_MINER) << "check_proof_of_stake: bits: " << target << ", header hash: " << pos;
    // TODO
    return true;
}



