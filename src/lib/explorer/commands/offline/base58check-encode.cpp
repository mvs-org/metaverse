/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
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
#include <metaverse/explorer/commands/base58check-encode.hpp>

#include <iostream>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/config/wrapper.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::config;
using namespace bc::explorer::config;

console_result base58check_encode::invoke(std::ostream& output, 
    std::ostream& error)
{
    // Bound parameters.
    const auto version = get_version_option();
    const auto& payload = get_base16_argument();

    const wrapper wrapped(version, payload);
    const auto encoded_wrapper = wrapped.to_data();
    const base58 base58check(encoded_wrapper);

    output << base58check << std::flush;
    return console_result::okay;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
