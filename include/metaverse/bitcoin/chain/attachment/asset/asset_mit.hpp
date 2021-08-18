/**
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_CHAIN_ATTACH_ASSET_MIT_HPP
#define MVS_CHAIN_ATTACH_ASSET_MIT_HPP

#include <cstdint>
#include <istream>
#include <set>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/error.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

namespace libbitcoin {
namespace chain {
class asset_mit;
}
}

#define MIT_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<bc::chain::asset_mit::mit_status>::type>(kd))

#define MIT_STATUS_NONE        MIT_STATUS2UINT32(bc::chain::asset_mit::mit_status::mit_status_none)
#define MIT_STATUS_REGISTER    MIT_STATUS2UINT32(bc::chain::asset_mit::mit_status::mit_status_register)
#define MIT_STATUS_TRANSFER    MIT_STATUS2UINT32(bc::chain::asset_mit::mit_status::mit_status_transfer)
#define MIT_STATUS_MAX         MIT_STATUS2UINT32(bc::chain::asset_mit::mit_status::mit_status_max)

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t ASSET_MIT_SYMBOL_FIX_SIZE  = 64;
BC_CONSTEXPR size_t ASSET_MIT_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_MIT_CONTENT_FIX_SIZE = 256;
BC_CONSTEXPR size_t ASSET_MIT_STATUS_FIX_SIZE  = 1;

BC_CONSTEXPR size_t ASSET_MIT_FIX_SIZE = (ASSET_MIT_SYMBOL_FIX_SIZE
        + ASSET_MIT_ADDRESS_FIX_SIZE + ASSET_MIT_CONTENT_FIX_SIZE
        + ASSET_MIT_STATUS_FIX_SIZE);

BC_CONSTEXPR size_t ASSET_MIT_TRANSFER_FIX_SIZE = (
            ASSET_MIT_FIX_SIZE - ASSET_MIT_CONTENT_FIX_SIZE);

// output_height; timestamp; to_did; mit;
BC_CONSTEXPR size_t ASSET_MIT_INFO_FIX_SIZE = 4 + 4 + 64 + ASSET_MIT_TRANSFER_FIX_SIZE;

class BC_API asset_mit
    : public base_primary<asset_mit>
{
public:
    typedef std::vector<asset_mit> list;

    enum class mit_status : uint8_t
    {
        mit_status_none     = 0,
        mit_status_register = 1,
        mit_status_transfer = 2,
        mit_status_max      = 3,
    };

    asset_mit();
    asset_mit(const std::string& symbol, const std::string& address,
              const std::string& content);

    void reset();
    bool is_valid() const;
    bool operator< (const asset_mit& other) const;

    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;
    data_chunk to_short_data() const;

    std::string to_string() const;
    uint64_t serialized_size() const;
    uint64_t calc_size() const;
    uint64_t get_max_serialized_size() const;

    const std::string& get_symbol() const;
    void set_symbol(const std::string& symbol);

    const std::string& get_address() const;
    void set_address(const std::string& address);

    void set_content(const std::string& content);
    const std::string& get_content() const;

    void set_status(uint8_t status);
    uint8_t get_status() const;
    std::string get_status_name() const;

    static std::string status_to_string(uint8_t status);

    bool is_register_status() const;
    bool is_transfer_status() const;
    bool is_invalid_status() const;

private:
    // NOTICE: ref CAssetMit in transaction.h
    // asset_mit and CAssetMit should have the same size and order.
    uint8_t status_;        // asset status
    std::string symbol_;    // asset name/symbol
    std::string address_;   // address that owned asset cert
    std::string content_;   // the content of the asset, only serialization in register.
};

struct BC_API asset_mit_info
{
    typedef std::vector<asset_mit_info> list;
    uint32_t output_height;
    uint32_t timestamp;
    std::string to_did;
    asset_mit mit;

    uint64_t serialized_size() const;
    void reset();
    bool operator< (const asset_mit_info& other) const;

    static asset_mit_info factory_from_data(reader& source);
    data_chunk to_data() const;
    data_chunk to_short_data() const;
};

} // namespace chain
} // namespace libbitcoin

#endif

