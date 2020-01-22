/**
 * Copyright (c) 2011-2020 metaverse developers (see AUTHORS)
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
#pragma once

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/output_point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/chain/attachment/did/did_detail.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

namespace libbitcoin {
namespace chain {

class BC_API blockchain_did
    : public base_primary<blockchain_did>
{
public:
    enum did_address_type: uint32_t
    {
        address_invalid,
        address_current,
        address_history
    };
    typedef std::vector<blockchain_did> list;
    blockchain_did();
    blockchain_did( uint32_t version, const output_point& tx_point,
            uint64_t height, uint32_t status, const did_detail& did);
    static uint64_t satoshi_fixed_size();

    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;

#ifdef MVS_DEBUG
    std::string to_string() const;
#endif

    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    const uint32_t& get_version() const;
    void set_version(const uint32_t& version_);
    const output_point& get_tx_point() const;
    void set_tx_point(const output_point& tx_point_);
    const uint64_t& get_height() const;
    void set_height(const uint64_t& height_);
    const did_detail& get_did() const;
    void set_did(const did_detail& did_);
    void set_status(const uint32_t & status);
    const uint32_t& get_status() const;
    std::string get_status_string() const;
private:
    uint32_t version_;
    output_point tx_point_;
    uint64_t height_;
    uint32_t status_;
    did_detail did_;
};

} // namespace chain
} // namespace libbitcoin
