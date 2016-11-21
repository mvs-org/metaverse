#include <bitcoin/consensus/libdevcore/BasicType.h>

using namespace dev;

/*****************************/
WorkPackage::WorkPackage(libbitcoin::chain::header& _bh):
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

HeaderAux* dev::HeaderAux::s_this = nullptr;


HeaderAux* HeaderAux::get()
{
	static std::once_flag flag;
	std::call_once(flag, []{s_this = new HeaderAux();});
	return s_this;
}


h256 HeaderAux::seedHash(libbitcoin::chain::header& _bi)
{
	unsigned _number = (unsigned) _bi.getNumber();
	unsigned epoch = _number / ETHASH_EPOCH_LENGTH;
	Guard l(get()->x_epochs);
	if (epoch >= get()->m_seedHashes.size())
	{
		h256 ret;
		unsigned n = 0;
		if (!get()->m_seedHashes.empty())
		{
			ret = get()->m_seedHashes.back();
			n = get()->m_seedHashes.size() - 1;
		}
		get()->m_seedHashes.resize(epoch + 1);
//		cdebug << "Searching for seedHash of epoch " << epoch;
		for (; n <= epoch; ++n, ret = sha3(ret))
		{
			get()->m_seedHashes[n] = ret;
//			cdebug << "Epoch" << n << "is" << ret;
		}
	}
	return get()->m_seedHashes[epoch];
}

uint64_t HeaderAux::number(h256& _seedHash)
{
	Guard l(get()->x_epochs);
	unsigned epoch = 0;
	auto epochIter = get()->m_epochs.find(_seedHash);
	if (epochIter == get()->m_epochs.end())
	{
		//		cdebug << "Searching for seedHash " << _seedHash;
		for (h256 h; h != _seedHash && epoch < 2048; ++epoch, h = sha3(h), get()->m_epochs[h] = epoch) {}
		if (epoch == 2048)
		{
			std::ostringstream error;
			error << "apparent block number for " << _seedHash << " is too high; max is " << (ETHASH_EPOCH_LENGTH * 2048);
			throw std::invalid_argument(error.str());
		}
	}
	else
		epoch = epochIter->second;
	return epoch * ETHASH_EPOCH_LENGTH;
}

h256 HeaderAux::hashHead(libbitcoin::chain::header& _bi)
{
	h256 memo;
	RLPStream s;
	s  << (bigint) _bi.version << (bigint)_bi.bits << (bigint)_bi.m_number << (char*)(_bi.merkle.data())
		<< (char*)(_bi.previous_block_hash.data()) << (bigint) _bi.timestamp ;
	memo = sha3(s.out());
	return memo;
}

uint64_t HeaderAux::cacheSize(libbitcoin::chain::header& _header)
{
	return ethash_get_cachesize((uint64_t)_header.m_number);
}

uint64_t HeaderAux::dataSize(uint64_t _blockNumber)
{
	return ethash_get_datasize(_blockNumber);
}


/*****************************/

ChainOperationParams::ChainOperationParams()
{
	otherParams = std::unordered_map<std::string, std::string>{
		{"minGasLimit", "0x1388"},
		{"maxGasLimit", "0x7fffffffffffffff"},
		{"gasLimitBoundDivisor", "0x0400"},
		{"minimumDifficulty", "0x020000"},
		{"difficultyBoundDivisor", "0x0800"},
		{"durationLimit", "0x0d"},
		{"registrar", "5e70c0bbcd5636e0f9f9316e9f8633feb64d4050"},
		{"networkID", "0x0"}
	};
	blockReward = u256("0x4563918244F40000");
}

u256 ChainOperationParams::u256Param(std::string const& _name)
{
	std::string at("");

	auto it = otherParams.find(_name);
	if (it != otherParams.end())
		at = it->second;

	return u256(fromBigEndian<u256>(fromHex(at)));
}
