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
#ifndef MVS_CHAIN_ATTACH_ASSET_HPP
#define MVS_CHAIN_ATTACH_ASSET_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/lib/bitcoin/chain/point.hpp>
#include <metaverse/lib/bitcoin/chain/script/script.hpp>
#include <metaverse/lib/bitcoin/define.hpp>
#include <metaverse/lib/bitcoin/utility/reader.hpp>
#include <metaverse/lib/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <metaverse/lib/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <metaverse/lib/bitcoin/chain/attachment/asset/asset_transfer.hpp>

using namespace libbitcoin::chain;

#define ASSET_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<asset::asset_status>::type>(kd))

#define ASSET_DETAIL_TYPE ASSET_STATUS2UINT32(asset::asset_status::asset_locked)
#define ASSET_TRANSFERABLE_TYPE ASSET_STATUS2UINT32(asset::asset_status::asset_transferable)

namespace libbitcoin {
namespace chain {

class BC_API asset
{
public:
	enum class asset_status : uint32_t
	{
		asset_none,
		asset_locked,
		asset_transferable,
	};
	typedef boost::variant<asset_detail, asset_transfer> asset_data_type;

	asset();
	asset(uint32_t status, const asset_detail& detail);
	asset(uint32_t status, const asset_transfer& detail);
    static asset factory_from_data(const data_chunk& data);
    static asset factory_from_data(std::istream& stream);
    static asset factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    std::string to_string() const;
	bool is_valid_type() const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
	uint32_t get_status() const;
	void set_status(uint32_t status);
	void set_data(const asset_detail& detail);
	void set_data(const asset_transfer& detail);
	asset_data_type& get_data();
	
private:
    uint32_t status;
    asset_data_type data;

};

} // namespace chain
} // namespace libbitcoin

#endif

