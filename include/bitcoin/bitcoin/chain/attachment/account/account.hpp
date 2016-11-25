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
#ifndef LIBBITCOIN_CHAIN_ATTACHMENT_ACCOUNT_HPP
#define LIBBITCOIN_CHAIN_ATTACHMENT_ACCOUNT_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <bitcoin/bitcoin/chain/point.hpp>
#include <bitcoin/bitcoin/chain/script/script.hpp>
#include <bitcoin/bitcoin/define.hpp>
#include <bitcoin/bitcoin/utility/reader.hpp>
#include <bitcoin/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace chain {
/// used for store account related information 
class BC_API account
{
public:
	account();
	account(std::string name, std::string mnemonic, hash_digest passwd, 
			uint32_t hd_index, uint8_t priority);
    static account factory_from_data(const data_chunk& data);
    static account factory_from_data(std::istream& stream);
    static account factory_from_data(reader& source);
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
	operator bool() const;

    std::string name;
    std::string mnemonic;
    hash_digest passwd;
	
    uint32_t hd_index;
	uint8_t  priority;
};

} // namespace chain
} // namespace libbitcoin

#endif

