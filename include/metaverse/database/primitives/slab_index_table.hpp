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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_DATABASE_SLAB_INDEX_TABLE_HPP
#define MVS_DATABASE_SLAB_INDEX_TABLE_HPP

#include <metaverse/database/primitives/slab_hash_table.hpp>

namespace libbitcoin {
    namespace database {
        template <typename KeyType, typename IndexType>
        class slab_index_table : public slab_hash_table<KeyType>
        {
        public:
            slab_index_table(slab_hash_table_header& header, slab_manager& manager);

            const IndexType allocate_index();
            void free_index(const IndexType& index );

            //set capability of result to limit the max slab num to return
            void finds_by_index(const IndexType& index, std::vector<memory_ptr> &result) const;
        private:
            // Override this function to specify index by key without hash calc.
            virtual array_index bucket_index(const KeyType& key) const final ;

            // faster index allocation
            IndexType last_index = 0;
            slab_manager& manager__;
            slab_hash_table_header& header__;
            shared_mutex index_mutex_;
        };

    } // namespace database
} // namespace libbitcoin

#include <metaverse/database/impl/slab_index_table.ipp>

#endif
