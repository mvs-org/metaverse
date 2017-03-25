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

#include <metaverse/node.hpp>
#include <metaverse/protocol.hpp>
#include <metaverse/server/configuration.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/parser.hpp>
#include <metaverse/server/server_node.hpp>
#include <metaverse/server/settings.hpp>
#include <metaverse/server/version.hpp>
#include <metaverse/server/interface/address.hpp>
#include <metaverse/server/interface/blockchain.hpp>
#include <metaverse/server/interface/protocol.hpp>
#include <metaverse/server/interface/transaction_pool.hpp>
#include <metaverse/server/messages/message.hpp>
#include <metaverse/server/messages/route.hpp>
#include <metaverse/server/services/block_service.hpp>
#include <metaverse/server/services/heartbeat_service.hpp>
#include <metaverse/server/services/query_service.hpp>
#include <metaverse/server/services/transaction_service.hpp>
#include <metaverse/server/utility/address_key.hpp>
#include <metaverse/server/utility/authenticator.hpp>
#include <metaverse/server/utility/fetch_helpers.hpp>
#include <metaverse/server/workers/notification_worker.hpp>
#include <metaverse/server/workers/query_worker.hpp>

#endif
