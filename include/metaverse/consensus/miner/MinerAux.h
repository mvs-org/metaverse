#pragma once

#include <condition_variable>
#include <thread>
#include <metaverse/consensus/libethash/ethash.h>
#include <metaverse/consensus/libdevcore/Log.h>
#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/bitcoin/chain/output_point.hpp>
#include <metaverse/consensus/libdevcore/FixedHash.h>
#include <metaverse/consensus/libdevcore/Guards.h>
namespace libbitcoin
{


class MinerAux
{
public:
    ~MinerAux();
    static MinerAux* get();
    //static h256 mixHash(chain::header& _bi) { return _bi;}
    static void setNonce(chain::header& _bi, Nonce _v){_bi.nonce = (FixedHash<8>::Arith)_v; }
    static void setMixHash(chain::header& _bi, h256& _v){_bi.mixhash = (FixedHash<32>::Arith)_v; }
    static LightType get_light(h256& _seedHash);
    static FullType get_full(h256& _seedHash);
    static bool search(chain::header& header, std::function<bool (void)> is_exit);
    static uint64_t getRate(){ return get()->m_rate; }

    static bool verify_work(const chain::header& header, const chain::header::ptr parent);
    static bool verify_stake(const chain::header& header, const chain::output_info& stake_output);
    static bool check_proof_of_stake(const chain::header& header, const chain::output_info& stake_output);

private:
    MinerAux() {m_rate = 0;}
    static MinerAux* s_this;
    SharedMutex x_lights;
    std::unordered_map<h256, std::shared_ptr<LightAllocation>> m_lights;
    Mutex x_fulls;
    std::condition_variable m_fullsChanged;
    std::unordered_map<h256, std::weak_ptr<FullAllocation>> m_fulls;
    FullType m_lastUsedFull;
   // uint64_t m_hashCount;
    uint64_t m_rate;




};
}
