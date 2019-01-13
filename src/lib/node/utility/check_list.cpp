/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/node/utility/check_list.hpp>

#include <cstddef>
#include <utility>
#include <boost/bimap/support/lambda.hpp>
#include <metaverse/blockchain.hpp>

namespace libbitcoin {
namespace node {

using namespace bc::database;

bool check_list::empty() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return checks_.empty();
    ///////////////////////////////////////////////////////////////////////////
}

size_t check_list::size() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return checks_.size();
    ///////////////////////////////////////////////////////////////////////////
}

void check_list::reserve(const block_database::heights& heights)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    checks_.clear();

    for (const auto height: heights)
        const auto it = checks_.insert({ null_hash, height });

    ///////////////////////////////////////////////////////////////////////////
}

void check_list::enqueue(hash_digest&& hash, size_t height)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    using namespace boost::bimaps;
    const auto it = checks_.right.find(height);

    // Ignore the entry if it is not reserved.
    if (it == checks_.right.end())
        return;

    BITCOIN_ASSERT(it->second == null_hash);
    checks_.right.modify_data(it, _data = std::move(hash));
    ///////////////////////////////////////////////////////////////////////////
}

bool check_list::dequeue(hash_digest& out_hash, size_t& out_height)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    // Overlocking to reduce code in the dominant path.
    if (checks_.empty())
        return false;

    auto it = checks_.right.begin();
    out_height = it->first;
    out_hash = it->second;
    checks_.right.erase(it);
    return true;
    ///////////////////////////////////////////////////////////////////////////
}

} // namespace node
} // namespace libbitcoin
