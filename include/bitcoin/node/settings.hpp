/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-node.
 *
 * libbitcoin-node is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_NODE_SETTINGS_HPP
#define LIBBITCOIN_NODE_SETTINGS_HPP

#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

/// Common database configuration settings, properties not thread safe.
class BCN_API settings
{
public:
    settings();
    settings(bc::settings context);

    /// Properties.
    uint32_t block_timeout_seconds;
    uint32_t download_connections;
    bool transaction_pool_refresh;
};

} // namespace node
} // namespace libbitcoin

#endif
