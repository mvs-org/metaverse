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
#ifndef MVS_CHAIN_ATTACHMENT_ASSET_TRANSFER_HPP
#define MVS_CHAIN_ATTACHMENT_ASSET_TRANSFER_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <tuple>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

namespace libbitcoin {
namespace chain {

BC_CONSTEXPR size_t ASSET_TRANSFER_SYMBOL_FIX_SIZE = 64;
BC_CONSTEXPR size_t ASSET_TRANSFER_QUANTITY_FIX_SIZE = 8;

BC_CONSTEXPR size_t ASSET_TRANSFER_FIX_SIZE = ASSET_TRANSFER_SYMBOL_FIX_SIZE + ASSET_TRANSFER_QUANTITY_FIX_SIZE;

struct asset_balances {
    typedef std::vector<asset_balances> list;
    std::string symbol;
    std::string address;
    uint64_t unspent_asset;
    uint64_t locked_asset;

    // for sort
    bool operator< (const asset_balances& other) const;
};

struct asset_deposited_balance {
    asset_deposited_balance(const std::string& symbol_,
        const std::string& address_,
        const std::string& tx_hash_,
        uint64_t tx_height_)
        : symbol(symbol_)
        , address(address_)
        , tx_hash(tx_hash_)
        , tx_height(tx_height_)
        , unspent_asset(0)
        , locked_asset(0)
    {}

    std::string symbol;
    std::string address;
    std::string tx_hash;
    std::string model_param;
    uint64_t tx_height;
    uint64_t unspent_asset;
    uint64_t locked_asset;

    // for sort
    bool operator< (const asset_deposited_balance& other) const {
        typedef std::tuple<std::string, uint64_t> cmp_tuple;
        return cmp_tuple(symbol, tx_height) < cmp_tuple(other.symbol, other.tx_height);
    }

    typedef std::vector<asset_deposited_balance> list;
};

class BC_API asset_transfer
    : public base_primary<asset_transfer>
{
public:
    asset_transfer();
    asset_transfer(const std::string& symbol, uint64_t quantity);
    static uint64_t satoshi_fixed_size();

    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;

    std::string to_string() const;

    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    const std::string& get_symbol() const;
    void set_symbol(const std::string& symbol);
    uint64_t get_quantity() const;
    void set_quantity(uint64_t quantity);

private:
    std::string symbol;  // symbol  -- in block
    uint64_t quantity;  // -- in block
};

} // namespace chain
} // namespace libbitcoin

using asset_transfer = libbitcoin::chain::asset_transfer;

#endif

