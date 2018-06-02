/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_CHAIN_ATTACH_IDENTIFIABLE_ASSET_HPP
#define MVS_CHAIN_ATTACH_IDENTIFIABLE_ASSET_HPP

#include <cstdint>
#include <istream>
#include <set>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/error.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>

#define IDENTIFIABLE_ASSET_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<identifiable_asset::identifiable_asset_status>::type>(kd))

#define IDENTIFIABLE_ASSET_NORMAL_TYPE      IDENTIFIABLE_ASSET_STATUS2UINT32(identifiable_asset::identifiable_asset_status::none)
#define IDENTIFIABLE_ASSET_REGISTER_TYPE    IDENTIFIABLE_ASSET_STATUS2UINT32(identifiable_asset::identifiable_asset_status::register)
#define IDENTIFIABLE_ASSET_TRANSFER_TYPE    IDENTIFIABLE_ASSET_STATUS2UINT32(identifiable_asset::identifiable_asset_status::transfer)

using mit_status = bc::chain::identifiable_asset::identifiable_asset_status;

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t IDENTIFIABLE_ASSET_SYMBOL_FIX_SIZE  = 64;
BC_CONSTEXPR size_t IDENTIFIABLE_ASSET_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t IDENTIFIABLE_ASSET_CONTENT_FIX_SIZE = 256;
BC_CONSTEXPR size_t IDENTIFIABLE_ASSET_STATUS_FIX_SIZE  = 1;

BC_CONSTEXPR size_t IDENTIFIABLE_ASSET_FIX_SIZE = (IDENTIFIABLE_ASSET_SYMBOL_FIX_SIZE
        + IDENTIFIABLE_ASSET_ADDRESS_FIX_SIZE + IDENTIFIABLE_ASSET_CONTENT_FIX_SIZE
        + IDENTIFIABLE_ASSET_STATUS_FIX_SIZE);

class BC_API identifiable_asset
{
public:
    typedef std::vector<identifiable_asset> list;

    enum class identifiable_asset_status : uint8_t
    {
        none   = 0,
        register = 1,
        transfer = 2,
    };

    identifiable_asset();
    identifiable_asset(const std::string& symbol, const std::string& address,
                       const std::string& content);

    void reset();
    bool is_valid() const;
    bool operator< (const identifiable_asset& other) const;

    static identifiable_asset factory_from_data(const data_chunk& data);
    static identifiable_asset factory_from_data(std::istream& stream);
    static identifiable_asset factory_from_data(reader& source);

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;

    std::string to_string() const;
    uint64_t serialized_size() const;

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

private:
    // NOTICE: ref CIdentifiableAsset in transaction.h
    // identifiable_asset and CIdentifiableAsset should have the same size and order.
    std::string symbol_;    // asset name/symbol
    std::string address_;   // address that owned asset cert
    std::string content_;   // the content of the asset
    uint8_t status_;        // asset status
};

} // namespace chain
} // namespace libbitcoin

#endif

