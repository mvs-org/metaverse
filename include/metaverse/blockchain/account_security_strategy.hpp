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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef METAVERSE_ACCOUNT_SECURITY_STRATEGY_HPP
#define METAVERSE_ACCOUNT_SECURITY_STRATEGY_HPP

#include <cstdint>
#include <string>

#include <metaverse/bitcoin/utility/thread.hpp>

namespace libbitcoin {
    namespace blockchain {
        enum class auth_type: uint8_t  {
            AUTH_PASSWD = 0,
            AUTH_LASTWD = 1,

            AUTH_TYPE_CNT,
        };

        struct AccountInfo {
            uint8_t counter;
            uint32_t lock_start;

            void lock();
        };

        class account_security_strategy {
        public:
            static account_security_strategy* get_instance();

            void check_locked(const std::string &account_name);
            void on_auth_passwd(const std::string &account_name, const bool &result);
            void on_auth_lastwd(const std::string &account_name, const bool &result);

        private:
            account_security_strategy(const uint8_t &passwd_max_try, const uint8_t &lastwd_max_try, const uint32_t &max_lock_time);
            virtual ~account_security_strategy();

            account_security_strategy(const account_security_strategy&) = delete;
            void operator=(const account_security_strategy&) = delete;

            void on_auth(const std::string &account_name, const bool &result, const auth_type &type);

            const uint8_t MAX_TRY[static_cast<uint8_t>(auth_type::AUTH_TYPE_CNT)];
            const uint32_t MAX_LOCK_TIME; //seconds

            std::map<const std::string, AccountInfo> acc_info_[static_cast<uint8_t>(auth_type::AUTH_TYPE_CNT)];

            mutable shared_mutex mutex_;

            static account_security_strategy* instance;
        };

    }
}
#endif //METAVERSE_ACCOUNT_SECURITY_STRATEGY_HPP
