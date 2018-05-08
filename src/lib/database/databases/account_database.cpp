/**
 * Copyright (c) 2011-2015 mvs developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#include <metaverse/database/databases/account_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

namespace {
    // copy from block_chain_imp.cpp
    hash_digest get_hash(const std::string& str)
    {
        data_chunk data(str.begin(), str.end());
        return sha256_hash(data);
    }
}

account_database::account_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : base_database(map_filename, mutex)
{
}

// Close does not call stop because there is no way to detect thread join.
account_database::~account_database()
{
    close();
}

void account_database::set_admin(const std::string& name, const std::string& passwd)
{
    // create admin account if not exists
    const auto hash = get_hash(name);
    if( nullptr == get(hash)) {
        account acc;
        acc.set_name(name);
        acc.set_passwd(passwd);
        acc.set_priority(account_priority::administrator);
        store(acc);
        sync();
    }
}

void account_database::store(const account& account)
{
    const auto& name = account.get_name();
    const auto hash = get_hash(name);
    auto&& result = get_account_result(hash);
    if (result) {
        auto detail = result.get_account_detail();
        // account exist -- check duplicate
        if (*detail == account) {
            return;
        }
        // account exist -- check hash conflict
        if (detail && (detail->get_name() != name)) {
            log::error("account_database") << detail->get_name()
                << " is already exist and has same hash "
                << encode_hash(hash) << " with this name "
                << name;
            return;
        }
        // account exist -- remove old value
        remove(hash);
        sync();
    }

    const auto acc_size = account.serialized_size();
    BITCOIN_ASSERT(acc_size <= max_size_t);
    const auto value_size = static_cast<size_t>(acc_size);

    auto write = [&account](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(account.to_data());
    };

    lookup_map_.store(hash, write, value_size);
}

std::shared_ptr<std::vector<account>> account_database::get_accounts() const
{
    auto vec_acc = std::make_shared<std::vector<account>>();
    for (size_t i = 0; i < get_bucket_count(); i++ ) {
        auto memo = lookup_map_.find(i);
        if (memo->size()) {
            const auto action = [&vec_acc](memory_ptr elem)
            {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(account::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

account_result account_database::get_account_result(const hash_digest& hash) const
{
    const auto memory = get(hash);
    return account_result(memory);
}

} // namespace database
} // namespace libbitcoin
