#include <bitcoin/consensus/libethash/internal.h>
#include <bitcoin/consensus/libdevcore/Guards.h>
#include <bitcoin/consensus/libdevcore/Log.h>
#include <bitcoin/consensus/libdevcore/SHA3.h>
#include <bitcoin/bitcoin/chain/header.hpp>
#include <boost/detail/endian.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <array>
#include <thread>
#include <bitcoin/consensus/miner/MinerAux.h>
#include <random>


using namespace libbitcoin;
using namespace std;

MinerAux* libbitcoin::MinerAux::s_this = nullptr;

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
		//get()->m_lastUsedFull = ret;
		return ret;
	}
	//s_dagCallback = _f;
	ret = make_shared<FullAllocation>(l->light, dagCallbackShim);
	DEV_GUARDED(get()->x_fulls)
	get()->m_fulls[_seedHash] = ret;
	return ret;
}
