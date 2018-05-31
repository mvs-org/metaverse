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
#include <metaverse/blockchain/account_security_strategy.hpp>
#include <chrono>


namespace libbitcoin {
    namespace blockchain {
        account_security_strategy* account_security_strategy::instance = nullptr;

        void AccountInfo::lock() {
            counter = 0;
            const uint32_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            lock_start = now;
        }

        account_security_strategy::account_security_strategy(const uint8_t &passwd_max_try, const uint8_t &lastwd_max_try, const uint32_t &max_lock_time):
            MAX_TRY{passwd_max_try, lastwd_max_try},
            MAX_LOCK_TIME(max_lock_time)
        {

        }

        account_security_strategy::~account_security_strategy()
        {

        }

        account_security_strategy* account_security_strategy::get_instance() {
            if (instance == nullptr) {
                instance = new account_security_strategy(10, 8, 30*60); // 10 times for password, 8 times for lastword, and 30 minutes for lock period
            }

            return instance;
        }

        void account_security_strategy::check_locked(const std::string &account_name){
            unique_lock lock(mutex_);
            for (auto &acc: acc_info_) {
                auto iter = acc.find(account_name);
                if (iter == acc.end()) {
                    continue;
                }
                if (iter->second.lock_start == 0) {
                    continue;
                }

                const uint32_t now = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                if (iter->second.lock_start + MAX_LOCK_TIME > now) {
                    throw std::logic_error{"account locked, retry 30 minutes later."};
                }

                acc.erase(iter);
            }
        }

        void account_security_strategy::on_auth_passwd(const std::string &account_name, const bool &result){
            on_auth(account_name, result, auth_type::AUTH_PASSWD);
        }

        void account_security_strategy::on_auth_lastwd(const std::string &account_name, const bool &result){
            on_auth(account_name, result, auth_type::AUTH_LASTWD);
        }

        void account_security_strategy::on_auth(const std::string &account_name, const bool &result, const auth_type &type){
            unique_lock lock(mutex_);

            auto &acc = acc_info_[static_cast<uint8_t >(type)];

            if (result) {
                //auth success
                auto iter = acc.find(account_name);
                if ( iter == acc.end() ) {
                    return;
                }

                acc.erase(iter);

            } else {
                //auth fail
                auto iter = acc.find(account_name);
                if (iter == acc.end()) {
                    acc[account_name] = AccountInfo{1, 0};
                } else {
                    if (++(iter->second.counter) >= MAX_TRY[static_cast<uint8_t >(type)]) {
                        iter->second.lock();
                    }
                }
            }
        }

    }
}