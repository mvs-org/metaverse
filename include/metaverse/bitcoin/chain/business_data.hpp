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
#ifndef MVS_CHAIN_BUSINESS_DATA_HPP
#define MVS_CHAIN_BUSINESS_DATA_HPP

#include <cstdint>
#include <istream>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <boost/variant.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_cert.hpp>
#include <metaverse/bitcoin/chain/attachment/did/did_detail.hpp>
#include <metaverse/bitcoin/chain/attachment/etp/etp.hpp>
#include <metaverse/bitcoin/chain/attachment/etp/etp_award.hpp>
#include <metaverse/bitcoin/chain/attachment/message/message.hpp>

#define KIND2UINT16(kd)  (static_cast<typename std::underlying_type<business_kind>::type>(kd))
// 0 -- unspent  1 -- confirmed  2 -- local asset not issued
#define ASSET_STATUS_UNSPENT    0 // in blockchain
#define ASSET_STATUS_CONFIRMED  1 // in blockchain
#define ASSET_STATUS_UNISSUED   2 // in local database

namespace libbitcoin {
namespace chain {

enum class business_kind : uint16_t
{
    etp = 0,
    asset_issue = 1,
    asset_transfer = 2,
    message = 3,
    etp_award = 4, // store to address_asset database
    did_register = 5,
    did_transfer = 6,
    asset_cert = 7,
    unknown = 0xffff
};

// 0 -- unspent  1 -- confirmed  2 -- local asset not issued
enum business_status : uint8_t
{
    unspent = 0, // in blockchain but unspent
    confirmed = 1, // in blockchain confirmed
    unissued = 2, //  in local database ,special for asset related business
    unknown = 0xff
};

class BC_API business_data
{
public:
    typedef boost::variant<
        etp,
        etp_award,
        asset_detail,
        asset_transfer,
        asset_cert,
        blockchain_message,
        did_detail> business_data_type;
    static business_data factory_from_data(const data_chunk& data);
    static business_data factory_from_data(std::istream& stream);
    static business_data factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() ;
    void to_data(std::ostream& stream) ;
    void to_data(writer& sink);
#if MVS_DEBUG
    std::string to_string() ;
#endif
    bool is_valid() const;
    bool is_valid_type() const;
    void reset();
    uint64_t serialized_size() ;
    business_kind get_kind_value() const;
    const business_data_type& get_data() const;
    uint32_t get_timestamp() const;

private:
    business_kind kind; // 2 size
    uint32_t timestamp; // 4 size
    business_data_type data;

};

/*************************************business assisant class begin******************************************/
class BC_API business_record
{
public:
    typedef std::vector<business_record> list;

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
    } val_chk_sum;

    business_data data;

#ifdef MVS_DEBUG
    // just used for debug code in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t kind = " << KIND2UINT16(kind)
            << "\t point = " << point.to_string() << "\n"
            << "\t height = " << height
            << "\t data = " << data.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_history
{
public:
    typedef std::vector<business_history> list;
    /// If there is no output this is null_hash:max.
    output_point output;
    uint64_t output_height;

    /// The satoshi value of the output.
    uint64_t value;

    /// If there is no spend this is null_hash:max.
    input_point spend;

    union
    {
        /// The height of the spend or max if no spend.
        uint64_t spend_height;

        /// During expansion this value temporarily doubles as a checksum.
        uint64_t temporary_checksum;
    };
    uint32_t status; // 0 -- unspend  1 -- confirmed
    business_data data;  // for output only

#ifdef MVS_DEBUG
    // just used for debug code in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t output = " << output.to_string()
            << "\t output_height = " << output_height
            << "\t value = " << value << "\n"
            << "\t spend = " << spend.to_string()
            << "\t data = " << data.to_string() << "\n";

        return ss.str();
    }
#endif
};
class BC_API business_address_asset
{
public:
    typedef std::vector<business_address_asset> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local asset not issued
    uint64_t quantity;
    asset_detail detail;

#ifdef MVS_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t quantity = " << quantity << "\n"
            << "\t detail = " << detail.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_asset_cert
{
public:
    typedef std::vector<business_address_asset_cert> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local asset not issued
    asset_cert certs;

#ifdef MVS_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t certs = " << certs.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_did
{
public:
    typedef std::vector<business_address_did> list;

    std::string  address;
    uint8_t status; // 0 -- unspent  1 -- confirmed  2 -- local asset not issued
    did_detail detail;

#ifdef MVS_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t detail = " << detail.to_string() << "\n";

        return ss.str();
    }
#endif
};

class BC_API business_address_message
{
public:
    typedef std::vector<business_address_message> list;

    std::string  address;
    uint8_t status;
    chain::blockchain_message msg;
#ifdef MVS_DEBUG
    // just used for unit test in block_chain_impl_test.cpp
    std::string to_string()
    {
        std::ostringstream ss;

        ss << "\t address = " << address
            << "\t status = " << status
            << "\t message = " << msg.to_string() << "\n";

        return ss.str();
    }
#endif
};

/****************************************business assisant class end*******************************************/

} // namespace chain
} // namespace libbitcoin

#endif

