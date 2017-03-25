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

#include <metaverse/database.hpp>

#ifdef WITH_CONSENSUS
#include <metaverse/consensus.hpp>
#endif

#include <metaverse/blockchain/block.hpp>
#include <metaverse/blockchain/block_chain.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/block_detail.hpp>
#include <metaverse/blockchain/block_fetcher.hpp>
#include <metaverse/blockchain/define.hpp>
#include <metaverse/blockchain/organizer.hpp>
#include <metaverse/blockchain/orphan_pool.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <metaverse/blockchain/simple_chain.hpp>
#include <metaverse/blockchain/transaction_pool.hpp>
#include <metaverse/blockchain/transaction_pool_index.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <metaverse/blockchain/validate_block_impl.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>
#include <metaverse/blockchain/version.hpp>

#endif
