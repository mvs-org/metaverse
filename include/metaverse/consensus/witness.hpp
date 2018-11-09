/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-consensus.
 *
 * metaverse-consensus is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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

#ifndef MVS_CONSENSUS_WITNESS_HPP
#define MVS_CONSENSUS_WITNESS_HPP

#include <set>
#include <metaverse/bitcoin.hpp>

// forward declaration
namespace libbitcoin {
    namespace node {
        class p2p_node;
    } // node
    namespace blockchain {
        class settings;
    } // blockchain
    namespace wallet {
        class payment_address;
    } // wallet
} // libbitcoin

namespace libbitcoin {
namespace consensus {

class witness
{
public:
    using p2p_node = libbitcoin::node::p2p_node;
    using settings = blockchain::settings;
    using witness_id = wallet::payment_address;
    using list = std::list<witness_id>;

public:
    witness(p2p_node& node);
    ~witness();

    // return a copy list
    list get_witness_list() const;
    list get_candidate_list() const;

    bool is_witness(const witness_id& id) const;

    // register as a candidate
    bool register_witness(const witness_id& id);
    bool unregister_witness(const witness_id& id);

    // signature
    static bool sign(endorsement& out, const ec_secret& secret, const header& h);
    static bool verify_sign(const endorsement& out, const data_chunk& public_key, const header& h);

private:
    static bool exists(const list&, const witness_id&);

    // generate a new epoch witness list
    bool update_witness_list(uint64_t height);

private:
    p2p_node& node_;
    const settings& setting_;
    list witness_list_;
    list candidate_list_;
    mutable upgrade_mutex mutex_;
};

} // consensus
} // libbitcoin

#endif
