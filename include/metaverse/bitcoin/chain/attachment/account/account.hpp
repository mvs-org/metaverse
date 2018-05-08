/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_CHAIN_ATTACHMENT_ACCOUNT_HPP
#define MVS_CHAIN_ATTACHMENT_ACCOUNT_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>

namespace libbitcoin {
namespace chain {

enum account_status : uint8_t
{
    //system status
    locked = 0,
    imported,
    normal,
    //use status
    login,
    logout,
    error,
};

enum account_priority : uint8_t
{
    // 0 -- admin user  1 -- common user
    administrator = 0,
    common_user = 1,
};

enum account_type : uint8_t
{
    common = 0,
    multisignature,
};
/// used for store account related information
class BC_API account_multisig {
public:
    account_multisig();
    account_multisig(uint32_t hd_index, uint8_t m, uint8_t n,
        std::vector<std::string>&& cosigner_pubkeys, std::string& pubkey);
    void set_hd_index(uint32_t index);
    uint32_t get_hd_index() const;
    inline void increase_hd_index(){ hd_index_++; };
    void set_index(uint32_t index);
    uint32_t get_index() const;
    void set_m(uint8_t m);
    uint8_t get_m() const;
    void set_n(uint8_t n);
    uint8_t get_n() const;
    std::vector<std::string>& get_cosigner_pubkeys();
    void set_cosigner_pubkeys(std::vector<std::string>&& cosigner_pubkeys);
    std::string get_pubkey() const;
    void set_pubkey(std::string& pubkey);
    std::string get_description() const;
    void set_description(std::string& description);
    std::string get_address() const;
    void set_address(std::string& address);
    bool from_data(reader& source);
    void to_data(writer& sink) const;
    uint64_t serialized_size() const;
    bool operator==(account_multisig& other) ;
    void reset();
#ifdef MVS_DEBUG
    std::string to_string();
#endif
    std::string get_multisig_script();

private:
    uint32_t hd_index_;
    uint32_t index_;
    uint8_t m_;
    uint8_t n_;
    std::string pubkey_;
    std::vector<std::string> cosigner_pubkeys_;
    std::string description_;
    std::string address_;
};

class BC_API account
{
public:
    account();
    account(const std::string& name, const std::string& mnemonic, const hash_digest& passwd,
        uint32_t hd_index, uint8_t priority, uint8_t status, uint8_t type);
    static account factory_from_data(const data_chunk& data);
    static account factory_from_data(std::istream& stream);
    static account factory_from_data(reader& source);
    static uint64_t satoshi_fixed_size();

    bool from_data(const data_chunk& data);
    bool from_data(std::istream& stream);
    bool from_data(reader& source);
    data_chunk to_data() const;
    void to_data(std::ostream& stream) const;
    void to_data(writer& sink) const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;
    operator bool() const;
    bool operator==(const account& other) const;

#ifdef MVS_DEBUG
    std::string to_string() ;
#endif

    const std::string& get_name() const;
    void set_name(const std::string& name);
    const std::string& get_mnemonic(std::string& passphrase, std::string& decry_output) const;
    const std::string& get_mnemonic() const;
    void set_mnemonic(const std::string& mnemonic, std::string& passphrase);
    void set_mnemonic(const std::string& mnemonic);
    const hash_digest& get_passwd() const;

    void set_passwd(const std::string& outside_passwd){
        //bc::decode_hash(passwd, outside_passwd);
        data_chunk data(outside_passwd.begin(), outside_passwd.end());
        set_passwd(sha256_hash(data));
    }
    void set_passwd(const hash_digest& passwd_hash){
        passwd = passwd_hash;
    }

    uint32_t get_hd_index() const;
    void set_hd_index(const uint32_t hd_index);

    void increase_hd_index(){hd_index++;};

    uint8_t get_status() const;
    void set_status(const uint8_t status);
    uint8_t get_priority() const;
    void set_priority(const uint8_t priority);
    void set_user_status(const uint8_t status);
    void set_system_status(const uint8_t status);
    uint8_t get_user_status() const;
    uint8_t get_system_status() const;
    void set_type(uint8_t type);
    uint8_t get_type() const;
    const std::vector<account_multisig>& get_multisig_vec() const;
    void set_multisig_vec(std::vector<account_multisig>&& multisig);
    bool is_multisig_exist(account_multisig& multisig);
    void set_multisig(account_multisig& multisig);
    void remove_multisig(account_multisig& multisig, uint16_t index = 0);
    bool get_multisig(account_multisig& multisig, uint16_t index = 0);
    bool get_multisig_by_address(account_multisig& multisig, std::string& addr);
    void modify_multisig(account_multisig& multisig);

private:
    std::string name;
    std::string mnemonic;
    hash_digest passwd;

    uint32_t hd_index;
    //uint16_t status; // old define
    uint8_t type;
    uint8_t status;
    uint8_t priority;

    // multisig fields
    std::vector<account_multisig> multisig_vec;
    //account_multisig multisig;
};


} // namespace chain
} // namespace libbitcoin

#endif

