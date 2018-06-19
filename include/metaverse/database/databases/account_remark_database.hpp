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
#include <metaverse/database/databases/base_database.hpp>

namespace libbitcoin {
    namespace database {
        class BCD_API account_remark_database {

            class BCD_API account_remark_base_database: public base_database
            {
            public:
                using base_database::base_database;

                slab_map& get_lookup_map() {return lookup_map_;};

                void remove(const hash_digest& hash) {lookup_map_.unlink(hash);};
            };

        public:
            /// Construct the database.
            account_remark_database(const boost::filesystem::path& lookup_filename,
                                    const boost::filesystem::path& rows_filename,
                                    std::shared_ptr<shared_mutex> mutex=nullptr);

            /// Close the database (all threads must first be stopped).
            ~account_remark_database();

            /// Initialize a new base database.
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

            void remove(const std::string& account_name);

            void store(const std::string& account_name, const hash_digest& tx_hash, const std::string& remark);

            const std::string get(const std::string& account_name, const hash_digest& tx_hash);

            const std::map<hash_digest, std::string> get_all(const std::string& account_name);

        private:
            account_remark_base_database account_remark_table;
            account_remark_base_database account_remark_rows;

            inline hash_digest get_hash(const std::string& str) const
            {
                data_chunk data(str.begin(), str.end());
                return sha256_hash(data);
            }
        };

    }
}

#endif //METAVERSE_ACCOUNT_REMARK_DATABASE_HPP
