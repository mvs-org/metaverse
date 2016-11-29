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

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/data_base.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/settings.hpp>
#include <bitcoin/database/version.hpp>
#include <bitcoin/database/databases/block_database.hpp>
#include <bitcoin/database/databases/history_database.hpp>
#include <bitcoin/database/databases/spend_database.hpp>
#include <bitcoin/database/databases/stealth_database.hpp>
#include <bitcoin/database/databases/transaction_database.hpp>
#include <bitcoin/database/memory/accessor.hpp>
#include <bitcoin/database/memory/allocator.hpp>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/memory/memory_map.hpp>
#include <bitcoin/database/primitives/hash_table_header.hpp>
#include <bitcoin/database/primitives/record_hash_table.hpp>
#include <bitcoin/database/primitives/record_list.hpp>
#include <bitcoin/database/primitives/record_manager.hpp>
#include <bitcoin/database/primitives/record_multimap.hpp>
#include <bitcoin/database/primitives/record_multimap_iterable.hpp>
#include <bitcoin/database/primitives/record_multimap_iterator.hpp>
#include <bitcoin/database/primitives/slab_hash_table.hpp>
#include <bitcoin/database/primitives/slab_manager.hpp>
#include <bitcoin/database/result/block_result.hpp>
#include <bitcoin/database/result/transaction_result.hpp>

#endif
