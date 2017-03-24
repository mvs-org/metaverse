///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-blockchain developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MVS_BLOCKCHAIN_HPP
#define MVS_BLOCKCHAIN_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile 
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <metaverse/lib/database.hpp>

#ifdef WITH_CONSENSUS
#include <metaverse/lib/consensus.hpp>
#endif

#include <metaverse/lib/blockchain/block.hpp>
#include <metaverse/lib/blockchain/block_chain.hpp>
#include <metaverse/lib/blockchain/block_chain_impl.hpp>
#include <metaverse/lib/blockchain/block_detail.hpp>
#include <metaverse/lib/blockchain/block_fetcher.hpp>
#include <metaverse/lib/blockchain/define.hpp>
#include <metaverse/lib/blockchain/organizer.hpp>
#include <metaverse/lib/blockchain/orphan_pool.hpp>
#include <metaverse/lib/blockchain/settings.hpp>
#include <metaverse/lib/blockchain/simple_chain.hpp>
#include <metaverse/lib/blockchain/transaction_pool.hpp>
#include <metaverse/lib/blockchain/transaction_pool_index.hpp>
#include <metaverse/lib/blockchain/validate_block.hpp>
#include <metaverse/lib/blockchain/validate_block_impl.hpp>
#include <metaverse/lib/blockchain/validate_transaction.hpp>
#include <metaverse/lib/blockchain/version.hpp>

#endif
