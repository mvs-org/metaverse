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
#ifndef MVS_CHAIN_OUTPUT_HPP
#define MVS_CHAIN_OUTPUT_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/error.hpp>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/chain/attachment/attachment.hpp> // added for asset issue/transfer
#include <metaverse/bitcoin/chain/attachment/did/did.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

// forward declaration
namespace libbitcoin {
namespace blockchain {
    class block_chain_impl;
}
}

namespace libbitcoin {
namespace chain {

class BC_API output
    : public base_primary<output>
{
public:
    output();
    output(output&& other);
    output(const output& other);

    output(uint64_t&& value, chain::script&& script, attachment&& attach_data);
    output(const uint64_t& value, const chain::script& script, const attachment& attach_data);
    output& operator=(output&& other);
    output& operator=(const output& other);
    typedef std::vector<output> list;

    static uint64_t satoshi_fixed_size();
    static bool is_valid_symbol(const std::string& symbol, uint32_t tx_version);
    static bool is_valid_did_symbol(const std::string& symbol,  bool check_sensitive = false);
    static bool is_valid_mit_symbol(const std::string& symbol,  bool check_sensitive = false);
    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;
    std::string to_string(uint32_t flags) const;
    bool is_valid() const;
    bool is_null() const;
    code check_attachment_address(bc::blockchain::block_chain_impl& chain) const;
    std::string get_script_address() const;
    void reset();
    uint64_t serialized_size() const;
    uint64_t get_asset_amount() const;
    std::string get_asset_symbol() const;
    std::string get_asset_issuer() const;
    std::string get_asset_address() const;
    std::string get_asset_cert_symbol() const;
    std::string get_asset_mit_symbol() const;
    std::string get_asset_cert_owner() const;
    std::string get_asset_cert_address() const;
    asset_cert_type get_asset_cert_type() const;
    const data_chunk& get_attenuation_model_param() const;
    uint32_t get_lock_sequence(uint32_t default_value=0) const;
    bool is_asset() const;
    bool is_asset_transfer() const;
    bool is_asset_issue() const;
    bool is_asset_secondaryissue() const;
    bool is_asset_mit() const;
    bool is_asset_mit_register() const;
    bool is_asset_mit_transfer() const;
    bool is_asset_cert() const;
    bool is_asset_cert_issue() const;
    bool is_asset_cert_transfer() const;
    bool is_asset_cert_autoissue() const;
    bool is_etp() const;
    bool is_etp_award() const;
    bool is_message() const;
    bool is_did() const;
    bool is_did_register() const;
    bool is_did_transfer() const;
    asset_detail get_asset_detail() const;
    asset_transfer get_asset_transfer() const;
    asset_cert get_asset_cert() const;
    asset_mit get_asset_mit() const;
    std::string get_did_symbol() const;
    std::string get_did_address() const;
    did get_did() const;

    uint64_t value;
    chain::script script;
    attachment attach_data; // added for asset issue/transfer
};


} // namespace chain
} // namespace libbitcoin

using output = libbitcoin::chain::output;

#endif
