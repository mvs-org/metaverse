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
#ifndef MVS_MESSAGE_COMPACT_BLOCK_HPP
#define MVS_MESSAGE_COMPACT_BLOCK_HPP

#include <istream>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/bitcoin/message/prefilled_transaction.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace message {

class BC_API compact_block
{
public:
    typedef std::shared_ptr<compact_block> ptr;
    typedef mini_hash short_id;
    typedef mini_hash_list short_id_list;

    static compact_block factory_from_data(uint32_t version,
        const data_chunk& data);
    static compact_block factory_from_data(uint32_t version,
        std::istream& stream);
    static compact_block factory_from_data(uint32_t version,
        reader& source);

    bool from_data(uint32_t version, const data_chunk& data);
    bool from_data(uint32_t version, std::istream& stream);
    bool from_data(uint32_t version, reader& source);
    data_chunk to_data(uint32_t version) const;
    void to_data(uint32_t version, std::ostream& stream) const;
    void to_data(uint32_t version, writer& sink) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size(uint32_t version) const;

    static const std::string command;
    static const uint32_t version_minimum;
    static const uint32_t version_maximum;

    chain::header header;
    uint64_t nonce;
    short_id_list short_ids;
    prefilled_transaction::list transactions;
};

} // namespace message
} // namespace libbitcoin

#endif
