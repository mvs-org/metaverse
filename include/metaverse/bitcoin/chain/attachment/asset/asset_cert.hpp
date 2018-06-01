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
#ifndef MVS_CHAIN_ATTACH_ASSET_CERT_HPP
#define MVS_CHAIN_ATTACH_ASSET_CERT_HPP

#include <cstdint>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/error.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>

#define ASSET_CERT_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<asset_cert::asset_cert_status>::type>(kd))

#define ASSET_CERT_NORMAL_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_normal)
#define ASSET_CERT_ISSUE_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_issue)
#define ASSET_CERT_TRANSFER_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_transfer)
#define ASSET_CERT_AUTOISSUE_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_autoissue)

// forward declaration
namespace libbitcoin {
namespace blockchain {
    class block_chain_impl;
}
}

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t ASSET_CERT_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_CERT_OWNER_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_CERT_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_CERT_TYPE_FIX_SIZE = 4;
BC_CONSTEXPR size_t ASSET_CERT_STATUS_FIX_SIZE = 1;

BC_CONSTEXPR size_t ASSET_CERT_FIX_SIZE = (ASSET_CERT_SYMBOL_FIX_SIZE
    + ASSET_CERT_OWNER_FIX_SIZE + ASSET_CERT_ADDRESS_FIX_SIZE
    + ASSET_CERT_TYPE_FIX_SIZE + ASSET_CERT_STATUS_FIX_SIZE);

using asset_cert_type = uint32_t;
namespace asset_cert_ns {
    constexpr asset_cert_type none          = 0;
    constexpr asset_cert_type issue         = 1;
    constexpr asset_cert_type domain        = 2;
    constexpr asset_cert_type naming        = 3;
}

class BC_API asset_cert
{
public:
    typedef std::vector<asset_cert> list;

    enum class asset_cert_status : uint8_t
    {
        asset_cert_normal = 0,
        asset_cert_issue = 1,
        asset_cert_transfer = 2,
        asset_cert_autoissue = 3,
    };

    asset_cert();
    asset_cert(const std::string& symbol, const std::string& owner,
        const std::string& address, asset_cert_type certs);

    void reset();
    bool is_valid() const;
    bool operator< (const asset_cert& other) const;

    static asset_cert factory_from_data(const data_chunk& data);
    static asset_cert factory_from_data(std::istream& stream);
    static asset_cert factory_from_data(reader& source);

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

    uint8_t get_status() const;
    void set_status(uint8_t status);
    bool is_newly_generated() const;

    void set_owner(const std::string& owner);
    const std::string& get_owner() const;

    void set_address(const std::string& owner);
    const std::string& get_address() const;

    asset_cert_type get_certs() const;
    void set_certs(asset_cert_type certs);

    asset_cert_type get_type() const;
    void set_type(asset_cert_type cert_type);
    std::string get_type_name() const;

    // auxiliary functions
    std::string get_key() const;

    static const std::map<asset_cert_type, std::string>& get_type_name_map();
    static std::string get_type_name(asset_cert_type cert_type);
    static bool test_certs(const std::vector<asset_cert_type>& total, const std::vector<asset_cert_type>& parts);
    static bool test_certs(const std::vector<asset_cert_type>& certs, asset_cert_type cert_type);

    static std::string get_domain(const std::string& symbol);
    static bool is_valid_domain(const std::string& domain);
    static std::string get_key(const std::string&symbol, asset_cert_type bit);

private:
    // NOTICE: ref CAssetCert in transaction.h
    // asset_cert and CAssetCert should have the same size and order.
    std::string symbol_; // asset name/symbol
    std::string owner_;  // asset cert owner, an digital identity
    std::string address_; // address that owned asset cert
    asset_cert_type cert_type_; // asset certs
    uint8_t status_;        // asset status
};

} // namespace chain
} // namespace libbitcoin

#endif

