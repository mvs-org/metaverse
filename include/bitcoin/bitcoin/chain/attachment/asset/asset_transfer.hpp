/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#ifndef MVS_CHAIN_ATTACHMENT_ASSET_TRANSFER_HPP
#define MVS_CHAIN_ATTACHMENT_ASSET_TRANSFER_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <bitcoin/bitcoin/chain/point.hpp>
#include <bitcoin/bitcoin/chain/script/script.hpp>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>
#include <bitcoin/bitcoin/chain/history.hpp>

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t ASSET_TRANSFER_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_TRANSFER_QUANTITY_FIX_SIZE = 8;

class BC_API asset_transfer
{
public:
	asset_transfer();
	asset_transfer(const std::string& address, uint64_t quantity);
    static asset_transfer factory_from_data(const data_chunk& data);
    static asset_transfer factory_from_data(std::istream& stream);
    static asset_transfer factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    std::string to_string() ;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
	void to_json(std::ostream& output) ;
	const std::string& get_address() const;
	void set_address(const std::string& address);
	uint64_t get_quantity() const;
	void set_quantity(uint64_t quantity);
	
private:
    std::string address;  // symbol  -- in block
    uint64_t quantity;  // -- in block
};

class BC_API asset_transfer_compact
{
public:
    typedef std::vector<asset_transfer_compact> list;

    // The type of point (output or spend).
    point_kind kind;

    /// The point that identifies the record.
    chain::point point;

    /// The height of the point.
    uint64_t height;

    union
    {
        /// If output, then satoshi value of output.
        uint64_t value;

        /// If spend, then checksum hash of previous output point
        /// To match up this row with the output, recompute the
        /// checksum from the output row with spend_checksum(row.point)
        uint64_t previous_checksum;
    };
	asset_transfer transfer; // only used when kind==point_kind::output
};

class BC_API asset_transfer_history
{
public:
    typedef std::vector<asset_transfer_history> list;

    /// If there is no output this is null_hash:max.
    output_point output;
    uint64_t output_height;

    /// The satoshi value of the output.
    uint64_t value;
	asset_transfer transfer;

    /// If there is no spend this is null_hash:max.
    input_point spend;

    union
    {
        /// The height of the spend or max if no spend.
        uint64_t spend_height;

        /// During expansion this value temporarily doubles as a checksum.
        uint64_t temporary_checksum;
    };
};


} // namespace chain
} // namespace libbitcoin

#endif

