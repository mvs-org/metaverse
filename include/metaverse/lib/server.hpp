///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-server developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MVS_SERVER_HPP
#define MVS_SERVER_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile 
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <metaverse/lib/node.hpp>
#include <metaverse/lib/protocol.hpp>
#include <metaverse/lib/server/configuration.hpp>
#include <metaverse/lib/server/define.hpp>
#include <metaverse/lib/server/parser.hpp>
#include <metaverse/lib/server/server_node.hpp>
#include <metaverse/lib/server/settings.hpp>
#include <metaverse/lib/server/version.hpp>
#include <metaverse/lib/server/interface/address.hpp>
#include <metaverse/lib/server/interface/blockchain.hpp>
#include <metaverse/lib/server/interface/protocol.hpp>
#include <metaverse/lib/server/interface/transaction_pool.hpp>
#include <metaverse/lib/server/messages/message.hpp>
#include <metaverse/lib/server/messages/route.hpp>
#include <metaverse/lib/server/services/block_service.hpp>
#include <metaverse/lib/server/services/heartbeat_service.hpp>
#include <metaverse/lib/server/services/query_service.hpp>
#include <metaverse/lib/server/services/transaction_service.hpp>
#include <metaverse/lib/server/utility/address_key.hpp>
#include <metaverse/lib/server/utility/authenticator.hpp>
#include <metaverse/lib/server/utility/fetch_helpers.hpp>
#include <metaverse/lib/server/workers/notification_worker.hpp>
#include <metaverse/lib/server/workers/query_worker.hpp>

#endif
