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
#ifndef MVS_DATABASE_SLAB_INDEX_TABLE_IPP
#define MVS_DATABASE_SLAB_INDEX_TABLE_IPP


namespace libbitcoin {
    namespace database {
        template <typename KeyType, typename IndexType>
        slab_index_table<KeyType, IndexType>::slab_index_table(slab_hash_table_header& header,
                                                  slab_manager& manager)
                : slab_hash_table<KeyType>(header, manager), header__(header), manager__(manager)
        {
        }


        template <typename KeyType, typename IndexType>
        array_index slab_index_table<KeyType, IndexType>::bucket_index(const KeyType& key) const
        {
            const auto bucket = from_little_endian<IndexType>(key.begin(), key.begin()+ sizeof(IndexType));
            BITCOIN_ASSERT(bucket < header__.size());
            return bucket;
        }

        template <typename KeyType, typename IndexType>
        const IndexType slab_index_table<KeyType, IndexType>::allocate_index(  )
        {
            unique_lock lock(index_mutex_);
            for (size_t i = 0; i < header__.size(); ++i) {
                const auto index = (last_index + i) % header__.size();
                if (header__.read(index) == header__.empty) {
                    last_index = index+1;
                    return index;
                }
            }

            return (IndexType)0xffffffffffffffff;
        }

        template <typename KeyType, typename IndexType>
        void slab_index_table<KeyType, IndexType>::free_index(const IndexType& index )
        {
            header__.write(index, header__.empty);
        }

        //set capability of result to limit the max slab num to return
        template <typename KeyType, typename IndexType>
        void slab_index_table<KeyType, IndexType>::finds_by_index(const IndexType& index, std::vector<memory_ptr> &result) const
        {
            // find first item
            auto current = header__.read(index);
            static_assert(sizeof(current) == sizeof(file_offset), "Invalid size");

            // Iterate through list...
            while (current != header__.empty)
            {
                const slab_row<KeyType> item(manager__, current);

                if(item.out_of_memory())
                    break;

                // Found.
                result.push_back( manager__.get(current + sizeof(IndexType)) );

                const auto previous = current;
                current = item.next_position();

                // This may otherwise produce an infinite loop here.
                // It indicates that a write operation has interceded.
                // So we must return gracefully vs. looping forever.
                if (previous == current)
                    break;
            }
        }

    } // namespace database
} // namespace libbitcoin

#endif
