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
#ifndef MVS_CHAIN_ATTACHMENT_ASSET_DETAIL_HPP
#define MVS_CHAIN_ATTACHMENT_ASSET_DETAIL_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/lib/bitcoin/chain/point.hpp>
#include <metaverse/lib/bitcoin/chain/script/script.hpp>
#include <metaverse/lib/bitcoin/define.hpp>
#include <metaverse/lib/bitcoin/utility/reader.hpp>
#include <metaverse/lib/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t ASSET_DETAIL_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_DETAIL_MAX_SUPPLY_FIX_SIZE = 8;
BC_CONSTEXPR size_t ASSET_DETAIL_ASSET_TYPE_FIX_SIZE = 4;
BC_CONSTEXPR size_t ASSET_DETAIL_ISSUER_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_DETAIL_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_DETAIL_DESCRIPTION_FIX_SIZE = 64;

BC_CONSTEXPR size_t ASSET_DETAIL_FIX_SIZE = ASSET_DETAIL_SYMBOL_FIX_SIZE
			+ ASSET_DETAIL_MAX_SUPPLY_FIX_SIZE
			+ ASSET_DETAIL_ASSET_TYPE_FIX_SIZE
			+ ASSET_DETAIL_ISSUER_FIX_SIZE
			+ ASSET_DETAIL_ADDRESS_FIX_SIZE
			+ ASSET_DETAIL_DESCRIPTION_FIX_SIZE;

class BC_API asset_detail
{
public:
	typedef std::vector<asset_detail> list;
	asset_detail();
	asset_detail(std::string symbol, uint64_t maximum_supply,
		uint32_t asset_type, std::string issuer,
		std::string address, std::string description);
    static asset_detail factory_from_data(const data_chunk& data);
    static asset_detail factory_from_data(std::istream& stream);
    static asset_detail factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;

#ifdef MVS_DEBUG
    std::string to_string() const;
	void to_json(std::ostream& out);
#endif

    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
	const std::string& get_symbol() const;
	void set_symbol(const std::string& symbol);
	uint64_t get_maximum_supply() const;
	void set_maximum_supply(uint64_t maximum_supply);
	uint32_t get_asset_type() const;
	void set_asset_type(uint32_t asset_type);
	const std::string& get_issuer() const;
	void set_issuer(const std::string& issuer);
	const std::string& get_address() const;
	void set_address(const std::string& address);
	const std::string& get_description() const;
	void set_description(const std::string& description);

private:    
    std::string symbol;
    uint64_t maximum_supply;
    uint32_t asset_type;
    std::string issuer; 
    std::string address;
    std::string description;
    //uint64_t issue_price;

    //restrict section
    //uint32_t number_of_decimal_point; //number of decimal point
    //life circle
    //uint64_t flag; //is_white_list/is_tx_backwards/is_require_approval
    
    // relationship section
    //double fee;
    //correlation asset
    //std::string authentication_organization; //authentication organization
};

} // namespace chain
} // namespace libbitcoin

#endif

