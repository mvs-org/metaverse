/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_CHAIN_TRANSACTION_HPP
#define MVS_CHAIN_TRANSACTION_HPP

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/chain/input.hpp>
#include <metaverse/bitcoin/chain/output.hpp>
#include <metaverse/bitcoin/math/elliptic_curve.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/thread.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

namespace libbitcoin {
namespace chain {

enum transaction_version {
    first = 1,   //the frist version
    check_output_script = 2,   //add check output script
    check_nova_testnet = 3,
    check_nova_feature = 4,
    max_version = 5
};

class BC_API transaction
    : public base_primary<transaction>
{
public:
    typedef std::vector<transaction> list;
    typedef std::shared_ptr<transaction> ptr;
    typedef std::vector<ptr> ptr_list;
    typedef std::vector<size_t> indexes;

    static uint64_t satoshi_fixed_size();

    transaction();
    transaction(const transaction& other);
    transaction(uint32_t version, uint32_t locktime, const input::list& inputs,
        const output::list& outputs);

    transaction(transaction&& other);
    transaction(uint32_t version, uint32_t locktime, input::list&& inputs,
        output::list&& outputs);

    /// This class is move assignable [but not copy assignable].
    transaction& operator=(transaction&& other);

    // TODO: eliminate blockchain transaction copies and then delete this.
    transaction& operator=(const transaction& other) /*= delete*/;

    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;
    std::string to_string(uint32_t flags) const;
    bool is_valid() const;
    void reset();
    hash_digest hash() const;

    // sighash_type is used by OP_CHECKSIG
    hash_digest hash(uint32_t sighash_type) const;
    bool is_coinbase() const;
    bool is_pos_genesis_tx(bool is_testnet) const;
    bool is_coinstake() const;
    bool is_final(uint64_t block_height, uint32_t block_time) const;
    bool is_locked(size_t block_height, uint32_t median_time_past) const;
    bool is_locktime_conflict() const;
    uint64_t total_output_value() const;
    uint64_t serialized_size() const;
    uint64_t total_output_transfer_amount() const;

    uint64_t legacy_sigops_count(bool accurate=true) const;
    static uint64_t legacy_sigops_count(const transaction::list& txs, bool accurate=true);

    bool has_asset_issue() const;
    bool has_asset_secondary_issue() const;
    bool has_asset_transfer() const;
    bool has_asset_cert() const;
    bool has_asset_mit_transfer() const;

    bool has_did_register() const;
    bool has_did_transfer() const;

    uint32_t version;
    uint32_t locktime;
    input::list inputs;
    output::list outputs;

protected:
    bool all_inputs_final() const;

private:
    mutable upgrade_mutex mutex_;
    mutable std::shared_ptr<hash_digest> hash_;
};

} // namespace chain
} // namespace libbitcoin

#endif
