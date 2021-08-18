/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_TRACK_HPP
#define MVS_TRACK_HPP

#include <atomic>
#include <cstddef>
#include <string>

#define CONSTRUCT_TRACK(class_name) \
    track<class_name>(#class_name)

template <class Shared>
class track
{
public:
    static std::atomic<size_t> instances;

protected:
    track(const std::string& class_name);
    ~track();

private:
    const std::string class_;
#ifndef NDEBUG
    std::size_t count_;
#endif
};

#include <metaverse/bitcoin/impl/utility/track.ipp>

#endif
