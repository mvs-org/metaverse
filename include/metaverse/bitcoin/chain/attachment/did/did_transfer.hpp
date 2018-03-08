/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#ifndef MVS_CHAIN_ATTACHMENT_DID_TRANSFER_HPP
#define MVS_CHAIN_ATTACHMENT_DID_TRANSFER_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/chain/history.hpp>

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t DID_TRANSFER_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t DID_TRANSFER_QUANTITY_FIX_SIZE = 8;

BC_CONSTEXPR size_t DID_TRANSFER_FIX_SIZE = DID_TRANSFER_ADDRESS_FIX_SIZE + DID_TRANSFER_QUANTITY_FIX_SIZE;

class BC_API did_transfer
{
public:
	did_transfer();
	did_transfer(const std::string& address, uint64_t quantity);
    static did_transfer factory_from_data(const data_chunk& data);
    static did_transfer factory_from_data(std::istream& stream);
    static did_transfer factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;

    std::string to_string() const;
	void to_json(std::ostream& output) ;

    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
	const std::string& get_address() const;
	void set_address(const std::string& address);
	uint64_t get_quantity() const;
	void set_quantity(uint64_t quantity);
	
private:
    std::string address;  // symbol  -- in block
    uint64_t quantity;  // -- in block
};

} // namespace chain
} // namespace libbitcoin

#endif

