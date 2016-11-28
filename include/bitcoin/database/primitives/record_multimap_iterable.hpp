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
#ifndef MVS_DATABASE_RECORD_MULTIMAP_ITERABLE_HPP
#define MVS_DATABASE_RECORD_MULTIMAP_ITERABLE_HPP

#include <bitcoin/database/define.hpp>
#include <bitcoin/database/primitives/record_list.hpp>
#include <bitcoin/database/primitives/record_multimap_iterator.hpp>

namespace libbitcoin {
namespace database {

/// Result of a multimap database query. This is a container wrapper allowing
/// the values to be iterated.
class BCD_API record_multimap_iterable
{
public:
    record_multimap_iterable(const record_list& records, array_index begin);

    record_multimap_iterator begin() const;
    record_multimap_iterator end() const;

private:
    array_index begin_;
    const record_list& records_;
};

} // namespace database
} // namespace libbitcoin

#endif
