/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_THREAD_HPP
#define MVS_THREAD_HPP

#include <memory>
#include <boost/thread.hpp>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {

enum class thread_priority
{
    high,
    normal,
    low,
    lowest
};

typedef boost::mutex unique_mutex;
typedef boost::shared_mutex shared_mutex;
typedef boost::upgrade_mutex upgrade_mutex;

typedef boost::mutex::scoped_lock scoped_lock;
typedef boost::unique_lock<shared_mutex> unique_lock;
typedef boost::shared_lock<shared_mutex> shared_lock;
typedef boost::upgrade_lock<shared_mutex> upgrade_lock;
typedef boost::upgrade_to_unique_lock<shared_mutex> upgrade_to_unique_lock;

BC_API void set_thread_priority(thread_priority priority);

} // namespace libbitcoin

#endif
