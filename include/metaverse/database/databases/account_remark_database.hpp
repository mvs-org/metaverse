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

#ifndef METAVERSE_ACCOUNT_REMARK_DATABASE_HPP
#define METAVERSE_ACCOUNT_REMARK_DATABASE_HPP
#include <boost/filesystem.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/primitives/slab_index_table.hpp>
#include <metaverse/database/primitives/slab_manager.hpp>

namespace libbitcoin {
    namespace database {
        static BC_CONSTEXPR size_t account_remark_key_size = 2 + 32 ; //32-tx_hash, 2-account_index
        typedef byte_array<account_remark_key_size> account_remark_key;

        class BCD_API account_remark_database {
        public:
            /// Construct the database.
            account_remark_database(const boost::filesystem::path& lookup_filename,
                                    std::shared_ptr<shared_mutex> mutex=nullptr);

            /// Close the database (all threads must first be stopped).
            ~account_remark_database();

            /// Initialize a new transaction database.
            bool create();

            /// Call before using the database.
            bool start();

            /// Call to signal a stop of current operations.
            bool stop();

            /// Call to unload the memory map.
            bool close();

            /// Synchronise storage with disk so things are consistent.
            /// Should be done at the end of every block write.
            void sync();

            const uint16_t allocate_index();

            void free_index(const uint16_t& index );

            void store(const uint16_t& account_index, const hash_digest& tx_hash, const std::string& remark);

            const std::string get(const uint16_t& account_index, const hash_digest& tx_hash) const;

            const std::map<hash_digest, std::string> get_all(const uint16_t& account_index) const;

        private:
            account_remark_key get_key(const uint16_t& account_index, const hash_digest& tx_hash) const {
                account_remark_key key;

                const auto &&acc_id = to_little_endian<uint16_t>(account_index);
                std::copy(acc_id.begin(), acc_id.end(), key.begin());
                std::copy(tx_hash.begin(), tx_hash.end(), key.begin() + acc_id.size());
                return key;
            };


            typedef slab_index_table<account_remark_key, uint16_t> slab_map;

            // Hash table used for looking up txs by hash.
            memory_map lookup_file_;
            slab_hash_table_header lookup_header_;
            slab_manager lookup_manager_;
            slab_map lookup_map_;

        };

    }
}

#endif //METAVERSE_ACCOUNT_REMARK_DATABASE_HPP
