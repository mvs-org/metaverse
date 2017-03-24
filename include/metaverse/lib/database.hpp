///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-database developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MVS_DATABASE_HPP
#define MVS_DATABASE_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile 
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/database/data_base.hpp>
#include <metaverse/lib/database/define.hpp>
#include <metaverse/lib/database/settings.hpp>
#include <metaverse/lib/database/version.hpp>
#include <metaverse/lib/database/databases/block_database.hpp>
#include <metaverse/lib/database/databases/history_database.hpp>
#include <metaverse/lib/database/databases/spend_database.hpp>
#include <metaverse/lib/database/databases/stealth_database.hpp>
#include <metaverse/lib/database/databases/transaction_database.hpp>
#include <metaverse/lib/database/memory/accessor.hpp>
#include <metaverse/lib/database/memory/allocator.hpp>
#include <metaverse/lib/database/memory/memory.hpp>
#include <metaverse/lib/database/memory/memory_map.hpp>
#include <metaverse/lib/database/primitives/hash_table_header.hpp>
#include <metaverse/lib/database/primitives/record_hash_table.hpp>
#include <metaverse/lib/database/primitives/record_list.hpp>
#include <metaverse/lib/database/primitives/record_manager.hpp>
#include <metaverse/lib/database/primitives/record_multimap.hpp>
#include <metaverse/lib/database/primitives/record_multimap_iterable.hpp>
#include <metaverse/lib/database/primitives/record_multimap_iterator.hpp>
#include <metaverse/lib/database/primitives/slab_hash_table.hpp>
#include <metaverse/lib/database/primitives/slab_manager.hpp>
#include <metaverse/lib/database/result/block_result.hpp>
#include <metaverse/lib/database/result/transaction_result.hpp>

#endif
