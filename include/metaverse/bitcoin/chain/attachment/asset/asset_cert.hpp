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
#include <metaverse/bitcoin/base_primary.hpp>
#include <metaverse/bitcoin/compat.h>

#ifdef __BIG_ENDIAN__
    #define ASSET_CERT_BIG_ENDIAN
#elif defined __LITTLE_ENDIAN__
    /* override */
#elif defined __BYTE_ORDER
    #if __BYTE_ORDER__ ==  __ORDER_BIG_ENDIAN__
        #define ASSET_CERT_BIG_ENDIAN
    #endif
#else /* !defined __LITTLE_ENDIAN__ */
    #include <endian.h> /* machine/endian.h */
    #if __BYTE_ORDER__ ==  __ORDER_BIG_ENDIAN__
        #define ASSET_CERT_BIG_ENDIAN
    #endif
#endif

namespace libbitcoin {
namespace chain {
class asset_cert;
union asset_cert_type;
}
}
using asset_cert = libbitcoin::chain::asset_cert;
using asset_cert_type = libbitcoin::chain::asset_cert_type;

#define ASSET_CERT_STATUS2UINT32(kd)  (static_cast<typename std::underlying_type<asset_cert::asset_cert_status>::type>(kd))

#define ASSET_CERT_NORMAL_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_normal)
#define ASSET_CERT_ISSUE_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_issue)
#define ASSET_CERT_TRANSFER_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_transfer)
#define ASSET_CERT_AUTOISSUE_TYPE ASSET_CERT_STATUS2UINT32(asset_cert::asset_cert_status::asset_cert_autoissue)

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t ASSET_CERT_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_CERT_OWNER_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_CERT_ADDRESS_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_CERT_TYPE_FIX_SIZE = 4;
BC_CONSTEXPR size_t ASSET_CERT_STATUS_FIX_SIZE = 1;
BC_CONSTEXPR size_t ASSET_CERT_CONTENT_FIX_SIZE = 64;

BC_CONSTEXPR size_t ASSET_CERT_FIX_SIZE = (ASSET_CERT_SYMBOL_FIX_SIZE
    + ASSET_CERT_OWNER_FIX_SIZE + ASSET_CERT_ADDRESS_FIX_SIZE
    + ASSET_CERT_TYPE_FIX_SIZE + ASSET_CERT_STATUS_FIX_SIZE);

BC_CONSTEXPR size_t ASSET_CERT_FULL_FIX_SIZE = ASSET_CERT_FIX_SIZE + ASSET_CERT_CONTENT_FIX_SIZE;

union asset_cert_type
{
    asset_cert_type(uint32_t mask_= 0)
        :mask(mask_)
    {
    }

    operator uint32_t()const
    {
        return mask;
    }

    bool is_costom() const
    {
        return bits.custom == 1;
    }

    bool has_content() const
    {
        return bits.content == 1;
    }

    struct {
#ifdef ASSET_CERT_BIG_ENDIAN
        uint32_t custom:1;
        uint32_t content:1;
        uint32_t reserved:10;
        uint32_t type:20;
#else
        uint32_t type:20;
        uint32_t reserved:10;
        uint32_t content:1;
        uint32_t custom:1;
#endif
    } bits;

    uint32_t mask;
};

std::istream& operator>>(std::istream& in, asset_cert_type& out);

namespace asset_cert_ns {
    const asset_cert_type none          = 0;
    const asset_cert_type issue         = 1;
    const asset_cert_type domain        = 2;
    const asset_cert_type naming        = 3;
    const asset_cert_type mining        = 0x60000000 + 4;
    const asset_cert_type witness       = 5;

    const asset_cert_type custom        = 0x80000000;
    const asset_cert_type custom_max    = 0x800fffff;
    const asset_cert_type marriage      = custom + 0;
    const asset_cert_type kyc           = custom + 1;
}

class BC_API asset_cert
    : public base_primary<asset_cert>
{
public:
    typedef std::vector<asset_cert> list;
    typedef std::map<std::string, double> mining_subsidy_param_t;
    typedef std::shared_ptr<mining_subsidy_param_t> mining_subsidy_param_ptr;

    static const std::string key_initial;
    static const std::string key_interval;
    static const std::string key_base;

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

    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;

    std::string to_string() const;
    uint64_t serialized_size() const;
    uint64_t calc_size() const;

    const std::string& get_symbol() const;
    void set_symbol(const std::string& symbol);

    uint8_t get_status() const;
    void set_status(uint8_t status);
    bool is_newly_generated() const;

    void set_owner(const std::string& owner);
    const std::string& get_owner() const;

    void set_address(const std::string& owner);
    const std::string& get_address() const;

    void set_content(const std::string& content);
    const std::string& get_content() const;

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
    static std::string get_key(const std::string& symbol, const asset_cert_type& bit);
    static std::string get_witness_key(const std::string& symbol);

    static bool has_content(asset_cert_type cert_type);
    bool has_content() const;
    bool check_mining_subsidy_param() const;
    mining_subsidy_param_ptr get_mining_subsidy_param() const;

    static std::vector<std::string> get_mining_subsidy_param_keys();
    static mining_subsidy_param_ptr parse_mining_subsidy_param(const std::string& param);

    bool is_primary_witness() const;
    bool is_secondary_witness() const;

    static std::string get_primary_witness_symbol(const std::string& symbol);
    static bool is_valid_primary_witness(const std::string& symbol);
    static bool is_valid_secondary_witness(const std::string& symbol);

    // witness cert index start at 1.
    static uint32_t get_primary_witness_index(const std::string& symbol);
    static uint32_t get_secondary_witness_index(const std::string& symbol);

private:
    static bool parse_uint32(const std::string& param, uint32_t& value);

private:
    // NOTICE: ref CAssetCert in transaction.h
    // asset_cert and CAssetCert should have the same size and order.
    std::string symbol_; // asset name/symbol
    std::string owner_;  // asset cert owner, an digital identity
    std::string address_; // address that owned asset cert
    asset_cert_type cert_type_; // asset certs
    uint8_t status_;        // asset status
    std::string content_;
};

} // namespace chain
} // namespace libbitcoin

namespace asset_cert_ns = libbitcoin::chain::asset_cert_ns;

#endif

