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
#ifndef MVS_CHAIN_BLOCK_HPP
#define MVS_CHAIN_BLOCK_HPP

#include <cstdint>
#include <istream>
#include <memory>
#include <string>
#include <vector>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/bitcoin/chain/transaction.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

namespace libbitcoin {
namespace chain {

class BC_API block
    : public base_primary<block>
{
public:
    typedef std::vector<block> list;
    typedef std::shared_ptr<block> ptr;
    typedef std::vector<ptr> ptr_list;
    typedef std::vector<size_t> indexes;

    static hash_digest generate_merkle_root(
        const transaction::list& transactions);
    static block genesis_mainnet();
    static block genesis_testnet();

    block();
    block(const block& other);
    block(const chain::header& header,
        const chain::transaction::list& transactions,
        const ec_signature& blocksig={},
        const ec_compressed& pubkey={});

    block(block&& other);
    block(chain::header&& header,
        chain::transaction::list&& transactions,
        ec_signature&& blocksig={},
        ec_compressed&& pubkey={});

    /// This class is move assignable but not copy assignable.
    block& operator=(block&& other);
    void operator=(const block&) = delete;

    bool from_data_t(reader& source, bool with_transaction_count = true);
    void to_data_t(writer& sink, bool with_transaction_count = true) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size(bool with_transaction_count = true) const;
    bool is_proof_of_stake() const;
    bool is_proof_of_work() const;
    bool is_proof_of_dpos() const;

    chain::header header;
    transaction::list transactions;

    ec_signature blocksig; // pos/dpos block only
    ec_compressed public_key; // dpos block only
};

} // namespace chain
} // namespace libbitcoin

#endif
