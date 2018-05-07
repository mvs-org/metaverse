/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <metaverse/bitcoin/chain/output.hpp>
#include <cctype>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/bitcoin/wallet/payment_address.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>

namespace libbitcoin {
namespace chain {

output output::factory_from_data(const data_chunk& data)
{
    output instance;
    instance.from_data(data);
    return instance;
}

output output::factory_from_data(std::istream& stream)
{
    output instance;
    instance.from_data(stream);
    return instance;
}

output output::factory_from_data(reader& source)
{
    output instance;
    instance.from_data(source);
    return instance;
}
bool output::is_valid_symbol(const std::string& symbol, uint32_t tx_version)
{
    if (symbol.empty())
        return false;
    // length check
    if (symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        return false;
    // char check
    for (const auto& i : symbol) {
        if (!(std::isalnum(i) || i=='.'))
            return false;
        if ((tx_version >= transaction_version::check_nova_feature)
                && (i != std::toupper(i))) {
            return false;
        }
    }
    // sensitive check
    if ((tx_version >= transaction_version::check_nova_feature)
            && bc::wallet::symbol::is_sensitive(symbol)) {
        return false;
    }
    return true;
}

bool output::is_valid_did_symbol(const std::string& symbol, uint32_t tx_version)
{
    if (symbol.empty())
        return false;
    // length check
    if (symbol.length() > DID_DETAIL_SYMBOL_FIX_SIZE)
        return false;
    // char check
    for (const auto& i : symbol) {
        if (!(std::isalnum(i) || i=='.'|| i=='@'))
            return false;
    }
    // sensitive check
    if (bc::wallet::symbol::is_sensitive(symbol)) {
        return false;
    }
    return true;
}

bool output::is_valid() const
{
    return (value != 0) || script.is_valid()
        || attach_data.is_valid(); // added for asset issue/transfer
}

std::string output::get_script_address() const
{
    auto payment_address = wallet::payment_address::extract(script);
    return payment_address.encoded();
}

code output::check_attachment_address(bc::blockchain::block_chain_impl& chain) const
{
    bool is_asset = false;
    bool is_did = false;
    std::string attachment_address;
    if (is_asset_issue() || is_asset_secondaryissue()) {
        attachment_address = get_asset_address();
        is_asset = true;
    } else if (is_asset_cert()) {
        attachment_address = get_asset_cert_address(chain);
        is_asset = true;
    } else if (is_did_issue() || is_did_transfer()) {
        attachment_address = get_did_address();
        is_did = true;
    }
    if (is_asset || is_did) {
        auto script_address = get_script_address();
        if (attachment_address != script_address) {
            if (is_asset)
                return error::asset_address_not_match;
            if (is_did)
                return error::did_address_not_match;
        }
    }
    return error::success;
}

code output::check_attachment_did_match_address(bc::blockchain::block_chain_impl& chain) const
{
    if (attach_data.get_version() == DID_ATTACH_VERIFY_VERSION ) {
        auto todid = attach_data.get_to_did();
        if (!todid.empty()) {
            auto address = get_script_address();
            if (todid != chain.get_did_from_address(address)) {
                return error::did_address_not_match;
            }
        }
    }

    return error::success;
}

void output::reset()
{
    value = 0;
    script.reset();
    attach_data.reset(); // added for asset issue/transfer
}

bool output::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool output::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool output::from_data(reader& source)
{
    reset();

    value = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result)
        result = script.from_data(source, true,
            script::parse_mode::raw_data_fallback);

    /* begin added for asset issue/transfer */
    if (result)
        result = attach_data.from_data(source);
    /* end added for asset issue/transfer */

    if (!result)
        reset();

    return result;
}

data_chunk output::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    log::debug("output::to_data") << "data.size=" << data.size();
    log::debug("output::to_data") << "serialized_size=" << serialized_size();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void output::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void output::to_data(writer& sink) const
{
    sink.write_8_bytes_little_endian(value);
    script.to_data(sink, true);
    /* begin added for asset issue/transfer */
    attach_data.to_data(sink);
    /* end added for asset issue/transfer */
}

uint64_t output::serialized_size() const
{
    return 8 + script.serialized_size(true)
        + attach_data.serialized_size(); // added for asset issue/transfer
}

std::string output::to_string(uint32_t flags) const
{
    std::ostringstream ss;

    ss << "\tvalue = " << value << "\n"
        << "\t" << script.to_string(flags) << "\n"
        << "\t" << attach_data.to_string() << "\n"; // added for asset issue/transfer

    return ss.str();
}

uint64_t output::get_asset_amount() const // for validate_transaction.cpp to calculate asset transfer amount
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_maximum_supply();
        }
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            auto trans_info = boost::get<asset_transfer>(asset_info.get_data());
            return trans_info.get_quantity();
        }
    }
    return 0;
}

bool output::is_asset_transfer() const
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        return (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE);
    }
    return false;
}

bool output::is_did_transfer() const
{
    if(attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        return (did_info.get_status() == DID_TRANSFERABLE_TYPE);
    }
    return false;
}

bool output::is_asset_issue() const
{
    if(attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if(asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return !detail_info.is_asset_secondaryissue();
        }
    }
    return false;
}

bool output::is_asset_secondaryissue() const
{
    if(attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if(asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.is_asset_secondaryissue();
        }
    }
    return false;
}

bool output::is_asset_cert() const
{
    return (attach_data.get_type() == ASSET_CERT_TYPE);
}

bool output::is_asset_cert_issue() const
{
    if (attach_data.get_type() == ASSET_CERT_TYPE) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        if (cert_info.get_status() == ASSET_CERT_ISSUE_TYPE) {
            return true;
        }
    }
    return false;
}

bool output::is_asset() const
{
    return (attach_data.get_type() == ASSET_TYPE);
}

bool output::is_did() const
{
    return (attach_data.get_type() == DID_TYPE);
}

bool output::is_etp() const
{
    return (attach_data.get_type() == ETP_TYPE);
}

bool output::is_etp_award() const
{
    return (attach_data.get_type() == ETP_AWARD_TYPE);
}

bool output::is_message() const
{
    return (attach_data.get_type() == MESSAGE_TYPE);
}

std::string output::get_asset_symbol() const // for validate_transaction.cpp to calculate asset transfer amount
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_symbol();
        }
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            auto trans_info = boost::get<asset_transfer>(asset_info.get_data());
            return trans_info.get_symbol();
        }
    } else if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_asset_issuer() const // for validate_transaction.cpp to calculate asset transfer amount
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_issuer();
        }
    }
    return std::string("");
}

std::string output::get_asset_address() const // for validate_transaction.cpp to verify asset address
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            auto detail_info = boost::get<asset_detail>(asset_info.get_data());
            return detail_info.get_address();
        }
    }
    return std::string("");
}

asset_cert output::get_asset_cert() const
{
    if (is_asset_cert()) {
        return boost::get<asset_cert>(attach_data.get_attach());
    }
    return asset_cert();
}

std::string output::get_asset_cert_symbol() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_symbol();
    }
    return std::string("");
}

std::string output::get_asset_cert_owner() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_owner();
    }
    return std::string("");
}

std::string output::get_asset_cert_address(bc::blockchain::block_chain_impl& chain) const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_address();
    }
    return std::string("");
}

asset_cert_type output::get_asset_cert_type() const
{
    if (is_asset_cert()) {
        auto cert_info = boost::get<asset_cert>(attach_data.get_attach());
        return cert_info.get_certs();
    }
    return asset_cert_ns::none;
}

bool output::is_did_issue() const
{
    if(attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        return (did_info.get_status() ==  DID_DETAIL_TYPE);
    }
    return false;
}

std::string output::get_did_symbol() const // for validate_transaction.cpp to calculate did transfer amount
{
    if (attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        auto detail_info = boost::get<did_detail>(did_info.get_data());
        return detail_info.get_symbol();

    }
    return std::string("");
}

std::string output::get_did_address() const // for validate_transaction.cpp to calculate did transfer amount
{
    if(attach_data.get_type() == DID_TYPE) {
        auto did_info = boost::get<did>(attach_data.get_attach());
        auto detail_info = boost::get<did_detail>(did_info.get_data());
        return detail_info.get_address();

    }
    return std::string("");
}

did output::get_did() const
{
    if(attach_data.get_type() == DID_TYPE) {
        return boost::get<did>(attach_data.get_attach());
    }
    return did();
}

asset_transfer output::get_asset_transfer() const
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
            return boost::get<asset_transfer>(asset_info.get_data());
        }
    }
    log::error("output::get_asset_transfer") << "Asset type is not asset_transfer_TYPE.";
    return asset_transfer();
}

asset_detail output::get_asset_detail() const
{
    if (attach_data.get_type() == ASSET_TYPE) {
        auto asset_info = boost::get<asset>(attach_data.get_attach());
        if (asset_info.get_status() == ASSET_DETAIL_TYPE) {
            return boost::get<asset_detail>(asset_info.get_data());
        }
    }
    log::error("output::get_asset_detail") << "Asset type is not ASSET_DETAIL_TYPE.";
    return asset_detail();
}

const data_chunk& output::get_attenuation_model_param() const
{
    BITCOIN_ASSERT(operation::is_pay_key_hash_with_attenuation_model_pattern(script.operations));
    return operation::get_model_param_from_pay_key_hash_with_attenuation_model(script.operations);
}

} // namspace chain
} // namspace libbitcoin
