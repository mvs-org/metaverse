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
#ifndef LIBBITCOIN_NODE_PARSER_HPP
#define LIBBITCOIN_NODE_PARSER_HPP

#include <ostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/node/define.hpp>
#include <bitcoin/node/configuration.hpp>

namespace libbitcoin {
namespace node {

/// Parse configurable values from environment variables, settings file, and
/// command line positional and non-positional options.
class BCN_API parser
  : public config::parser
{
public:
    parser(bc::settings context);
    parser(const configuration defaults);

    /// Parse all configuration into member settings.
    virtual bool parse(int argc, const char* argv[], std::ostream& error);

    /// Load command line options (named).
    virtual options_metadata load_options();

    /// Load command line arguments (positional).
    virtual arguments_metadata load_arguments();

    /// Load configuration file settings.
    virtual options_metadata load_settings();

    /// Load environment variable settings.
    virtual options_metadata load_environment();

    /// The populated configuration settings values.
    configuration configured;
};  

} // namespace node
} // namespace libbitcoin

#endif
