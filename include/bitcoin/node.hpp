///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-node developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LIBBITCOIN_NODE_HPP
#define LIBBITCOIN_NODE_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile 
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <bitcoin/blockchain.hpp>
#include <bitcoin/network.hpp>
#include <bitcoin/node/configuration.hpp>
#include <bitcoin/node/define.hpp>
#include <bitcoin/node/p2p_node.hpp>
#include <bitcoin/node/parser.hpp>
#include <bitcoin/node/settings.hpp>
#include <bitcoin/node/version.hpp>
#include <bitcoin/node/protocols/protocol_block_in.hpp>
#include <bitcoin/node/protocols/protocol_block_out.hpp>
#include <bitcoin/node/protocols/protocol_block_sync.hpp>
#include <bitcoin/node/protocols/protocol_header_sync.hpp>
#include <bitcoin/node/protocols/protocol_transaction_in.hpp>
#include <bitcoin/node/protocols/protocol_transaction_out.hpp>
#include <bitcoin/node/protocols/protocol_version_quiet.hpp>
#include <bitcoin/node/sessions/session_block_sync.hpp>
#include <bitcoin/node/sessions/session_header_sync.hpp>
#include <bitcoin/node/sessions/session_inbound.hpp>
#include <bitcoin/node/sessions/session_manual.hpp>
#include <bitcoin/node/sessions/session_outbound.hpp>
#include <bitcoin/node/utility/header_queue.hpp>
#include <bitcoin/node/utility/performance.hpp>
#include <bitcoin/node/utility/reservation.hpp>
#include <bitcoin/node/utility/reservations.hpp>

#endif
