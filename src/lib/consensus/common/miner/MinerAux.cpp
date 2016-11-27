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

ethash_return_value_t MinerAux::search(libbitcoin::chain::header& header, std::function<bool (void)> is_exit)
{
	auto tid = std::this_thread::get_id();
	static std::mt19937_64 s_eng((utcTime() + std::hash<decltype(tid)>()(tid)));
	uint64_t tryNonce = s_eng();
	ethash_return_value ethashReturn;
	FullType dag;
	unsigned hashCount = 1;
	h256 seed = HeaderAux::seedHash(header);
	h256 header_hash = HeaderAux::hashHead(header);
	h256 boundary = HeaderAux::boundary(header);

	dag = get_full(seed);



	for (; ; tryNonce++, hashCount++)
	{
		ethashReturn = ethash_full_compute(dag->full, *(ethash_h256_t*)header_hash.data(), tryNonce);
		h256 value = h256((uint8_t*)&ethashReturn.result, h256::ConstructFromPointer);
		if (value <= boundary )
			break;
		if(is_exit() == true)
		{
			ethashReturn.success = false;
			break;
		}
	}

	return ethashReturn;
}

bool MinerAux::verifySeal(libbitcoin::chain::header& _header)
{
	Result result;
	h256 seedHash = HeaderAux::seedHash(_header);
	h256 headerHash  = HeaderAux::hashHead(_header);
	Nonce nonce = _header.getNonce();
	DEV_GUARDED(get()->x_fulls)
	if (FullType dag = get()->m_fulls[seedHash].lock())
	{
		result = dag->compute(headerHash, nonce);
		return ( result.value <= HeaderAux::boundary(_header) && result.mixHash == _header.getMixhash() );
	}
	result = get()->get_light(seedHash)->compute(headerHash, nonce);
	return ( result.value <= HeaderAux::boundary(_header) && result.mixHash == _header.getMixhash() );
}






