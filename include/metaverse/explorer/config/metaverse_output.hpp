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
#ifndef BX_METAVERSE_OUTPUT_HPP
#define BX_METAVERSE_OUTPUT_HPP

#include <iostream>
#include <cstdint>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/bitcoin/chain/attachment/attachment.hpp>

using namespace bc::chain;

/* NOTE: don't declare 'using namespace foo' in headers. */

namespace libbitcoin {
namespace explorer {
namespace config {

/**
 * Serialization helper to convert between a base58-string:number and 
 * a vector of tx_metaverse_output_type.
 */
class BCX_API metaverse_output
{
public:

    /**
     * Default constructor.
     */
    metaverse_output();

    /**
     * Initialization constructor.
     * @param[in]  tuple  The value to initialize with.
     */
    metaverse_output(const std::string& tuple);

    /// Parsed properties
    uint64_t amount() const;
    uint64_t value() const;
    uint8_t version() const;
    const chain::script& script() const;
    const short_hash& pay_to_hash() const;
    const data_chunk& ephemeral_data() const;
    attachment& attach_data() ;
    /**
     * Overload stream in. Throws if input is invalid.
     * @param[in]   input     The input stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The input stream reference.
     */
    friend std::istream& operator>>(std::istream& input,
        metaverse_output& argument);

private:

    /**
     * The transaction metaverse_output state of this object.
     * This data is translated to an metaverse_output given expected version information.
     */
	uint64_t amount_;
    uint64_t value_; // etp value
    uint8_t version_;
    chain::script script_;
    short_hash pay_to_hash_;
    data_chunk ephemeral_data_;
	attachment attach_data_; // bussiness data
};

} // namespace explorer
} // namespace config
} // namespace libbitcoin

#endif
