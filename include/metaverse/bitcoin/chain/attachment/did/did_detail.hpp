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
#ifndef MVS_CHAIN_ATTACHMENT_DID_DETAIL_HPP
#define MVS_CHAIN_ATTACHMENT_DID_DETAIL_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t DID_DETAIL_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t DID_DETAIL_ADDRESS_FIX_SIZE = 64;

BC_CONSTEXPR size_t DID_DETAIL_FIX_SIZE = DID_DETAIL_SYMBOL_FIX_SIZE
            + DID_DETAIL_ADDRESS_FIX_SIZE;

class BC_API did_detail
{
public:
    typedef std::vector<did_detail> list;

    enum did_detail_type : uint32_t
    {
        created,
        issued_not_in_blockchain,
        issued_in_blockchain
    };

    did_detail();
    did_detail(const std::string& symbol, const std::string& address);

    static did_detail factory_from_data(const data_chunk& data);
    static did_detail factory_from_data(std::istream& stream);
    static did_detail factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();
    static std::string get_blackhole_did_symbol();
    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;

    bool operator< (const did_detail& other) const;
    std::string to_string() const;
    void to_json(std::ostream& out);

    bool is_valid() const;
    void reset();
    uint32_t count_size() const;
    uint64_t serialized_size() const;
    const std::string& get_symbol() const;
    void set_symbol(const std::string& symbol);
    const std::string& get_address() const;
    void set_address(const std::string& address);

private:
    std::string symbol;
    std::string address;
};

} // namespace chain
} // namespace libbitcoin

#endif

