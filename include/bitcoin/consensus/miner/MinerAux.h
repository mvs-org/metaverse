#pragma once

#include <condition_variable>
#include <thread>
#include <bitcoin/consensus/libethash/ethash.h>
#include <bitcoin/consensus/libdevcore/Log.h>
#include <bitcoin/consensus/libdevcore/BasicType.h>
#include <bitcoin/bitcoin/chain/header.hpp>
#include <bitcoin/consensus/libdevcore/FixedHash.h>
#include <bitcoin/consensus/libdevcore/Guards.h>
using namespace libbitcoin;
namespace dev
{


class MinerAux
{
public:
	~MinerAux();
	static MinerAux* get();
	static h256 mixHash(chain::header& _bi) { return _bi.getMixhash();}
	static chain::header& setNonce(chain::header& _bi, Nonce _v){_bi.setNonce(_v); return _bi;}
	static chain::header& setMixHash(chain::header& _bi, h256& _v){_bi.setMixHash(_v); return _bi;}
	static LightType get_light(h256& _seedHash);
	static FullType get_full(h256& _seedHash);
	static bool verifySeal(chain::header& header);
	static ethash_return_value_t search(libbitcoin::chain::header& header, std::function<bool (void)> is_exit);


private:
	MinerAux() {}
    static MinerAux* s_this;
    SharedMutex x_lights;
    std::unordered_map<h256, std::shared_ptr<LightAllocation>> m_lights;
    Mutex x_fulls;
    std::condition_variable m_fullsChanged;
    std::unordered_map<h256, std::weak_ptr<FullAllocation>> m_fulls;




};
}
