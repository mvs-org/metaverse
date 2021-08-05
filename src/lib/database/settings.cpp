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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/database/settings.hpp>

#include <boost/filesystem.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

settings::settings()
  : history_start_height(0),
    stealth_start_height(0),
    directory("database")
{
}

settings::settings(bc::settings context)
  : settings()
{
    switch (context)
    {
        case bc::settings::mainnet:
        {
            stealth_start_height = 350000;
            directory = default_directory = { "mainnet" };
            break;
        }

        case bc::settings::testnet:
        {
            stealth_start_height = 500000;
            directory = default_directory = { "testnet" };
            break;
        }

        default:
        case bc::settings::none:
        {
        }
    }
}

} // namespace database
} // namespace libbitcoin
