/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_NODE_SESSION_INBOUND_HPP
#define LIBBITCOIN_NODE_SESSION_INBOUND_HPP

#include <memory>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/network.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

/// Inbound connections session, thread safe.
class BCN_API session_inbound
  : public network::session_inbound
{
public:
    typedef std::shared_ptr<session_inbound> ptr;

    /// Construct an instance.
    session_inbound(network::p2p& network, blockchain::block_chain& blockchain,
        blockchain::transaction_pool& pool);

protected:
    /// Overridden to attach blockchain protocols.
    void attach_protocols(network::channel::ptr channel) override;

    blockchain::block_chain& blockchain_;
    blockchain::transaction_pool& pool_;
};

} // namespace node
} // namespace libbitcoin

#endif
