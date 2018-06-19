/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
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

#ifndef INCLUDE_BITCOIN_NODE_PROTOCOLS_PROTOCOL_MINER_HPP_
#define INCLUDE_BITCOIN_NODE_PROTOCOLS_PROTOCOL_MINER_HPP_

#include <atomic>
#include <cstddef>
#include <memory>
#include <metaverse/blockchain.hpp>
#include <metaverse/network.hpp>
#include <metaverse/node/define.hpp>

namespace libbitcoin
{

namespace node
{

class BCN_API protocol_miner:
        public network::protocol_events, track<protocol_miner>
{
public:
    using ptr = std::shared_ptr<protocol_miner>;
    using indexes = chain::point::indexes;
    using transaction_ptr = message::transaction_message::ptr;
    protocol_miner(network::p2p& network, network::channel::ptr channel
            , blockchain::block_chain& blockchain
            /*, blockchain::transaction_pool& pool*/);
    virtual void start();
private:
    void handle_stop(const code& ec);
//    bool handle_accept_transaction(const code&, const indexes&, transaction_ptr);
private:
    blockchain::block_chain& blockchain_;
//    blockchain::transaction_pool& pool_;
};

}// namespace node
}// namespace libbitcoin

#endif /* INCLUDE_BITCOIN_NODE_PROTOCOLS_PROTOCOL_MINER_HPP_ */
