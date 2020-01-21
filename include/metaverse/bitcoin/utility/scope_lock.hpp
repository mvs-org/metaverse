/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_SCOPE_LOCK_HPP
#define MVS_SCOPE_LOCK_HPP

#include <memory>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/thread.hpp>

namespace libbitcoin {

// This is just unique_lock with a ptr member type, for convenience.
class BC_API scope_lock
{
public:
    typedef std::shared_ptr<scope_lock> ptr;

    /// Lock using the specified mutex reference.
    scope_lock(shared_mutex& mutex);

    /// Unlock.
    ~scope_lock();

private:
    shared_mutex& mutex_;
};

} // namespace libbitcoin

#endif
