/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/utility/dispatcher.hpp>

#include <string>
#include <metaverse/bitcoin/utility/threadpool.hpp>
#include <metaverse/bitcoin/utility/work.hpp>

namespace libbitcoin {

dispatcher::dispatcher(threadpool& pool, const std::string& name)
  : heap_(pool, name)
{
}

size_t dispatcher::ordered_backlog()
{
    return heap_.ordered_backlog();
}

size_t dispatcher::unordered_backlog()
{
    return heap_.unordered_backlog();
}

size_t dispatcher::concurrent_backlog()
{
    return heap_.concurrent_backlog();
}

size_t dispatcher::combined_backlog()
{
    return heap_.combined_backlog();
}

} // namespace libbitcoin
