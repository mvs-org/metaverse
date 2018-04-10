/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-server.
 *
 * metaverse-server is free software: you can redistribute it and/or
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
//
// Created by czp on 18-3-30.
//
#include "cryptojs_impl.h"
#include "aes256_cbc.h"
#include "md5.h"
#include <metaverse/bitcoin/utility/random.hpp>
#include <metaverse/bitcoin/formats/base_64.hpp>
#include <boost/smart_ptr.hpp>

#define CONCAT(a,b) a.insert(a.end(), b.begin(), b.end())

namespace cryptojs {
    using libbitcoin::data_chunk;

    data_chunk MD5Hash(const data_chunk &data) {
        MD5 md5;
        md5.update(data.data(), data.size());
        md5.finalize();

        return data_chunk(md5.raw_digest(), md5.raw_digest() + 16);
    }

    void derive_key(const data_chunk &passphrase, const data_chunk &salt, data_chunk &key, data_chunk &iv) {
        const uint8_t key_size = 32;
        const uint8_t iv_size = 16;
        //const uint8_t iterations  = 1;

        data_chunk derivedKeyWords;
        data_chunk block;

        while (derivedKeyWords.size() < (key_size + iv_size)) {
            //block.insert(block.end(), passphrase.begin(), passphrase.end());
            //block.insert(block.end(), salt.begin(), salt.end());
            CONCAT(block, passphrase);
            CONCAT(block, salt);
            block = MD5Hash(block);
            //derivedKeyWords.insert(derivedKeyWords.end(), block.begin(), block.end());
            CONCAT(derivedKeyWords, block);
        }

        key.assign(derivedKeyWords.begin(), derivedKeyWords.begin() + key_size);
        iv.assign(derivedKeyWords.begin() + key_size, derivedKeyWords.end());
    }

    data_chunk Pkcs7_Padding(const data_chunk &data, const uint8_t &block_size = 16) {
        const uint8_t nPaddingBytes = block_size - (data.size() % block_size);
        data_chunk ret(data);
        for (auto i = 0; i < nPaddingBytes; ++i) {
            ret.push_back(nPaddingBytes);
        }
        return ret;
    }

    data_chunk aes256_cbc_encrypt(const data_chunk &key, const data_chunk &iv, const data_chunk &block) {

        aes256_cbc::AES_ctx context;
        aes256_cbc::AES_init_ctx_iv(&context, key.data(), iv.data());
        const auto BLOCK_SIZE = block.size();
        uint8_t *p = new uint8_t[BLOCK_SIZE];
        boost::shared_array <uint8_t> sa(p);
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            p[i] = block[i];
        }

        aes256_cbc::AES_CBC_encrypt_buffer(&context, p, BLOCK_SIZE);

        return data_chunk(p, p + BLOCK_SIZE);
    }

    data_chunk aes256_cbc_decrypt(const data_chunk &key, const data_chunk &iv, const data_chunk &block) {

        aes256_cbc::AES_ctx context;
        aes256_cbc::AES_init_ctx_iv(&context, key.data(), iv.data());
        const auto BLOCK_SIZE = block.size();
        uint8_t *p = new uint8_t[BLOCK_SIZE];
        boost::shared_array <uint8_t> sa(p);
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
            p[i] = block[i];
        }

        aes256_cbc::AES_CBC_decrypt_buffer(&context, p, BLOCK_SIZE);

        return data_chunk(p, p + BLOCK_SIZE);
    }

    std::string encrypt(const std::string &message, const std::string &passphrase_) {
        const auto nonce = bc::pseudo_random();

        data_chunk salt = {
                (uint8_t) (nonce >> (0 * 8)),
                (uint8_t) (nonce >> (1 * 8)),
                (uint8_t) (nonce >> (2 * 8)),
                (uint8_t) (nonce >> (3 * 8)),
                (uint8_t) (nonce >> (4 * 8)),
                (uint8_t) (nonce >> (5 * 8)),
                (uint8_t) (nonce >> (6 * 8)),
                (uint8_t) (nonce >> (7 * 8)),
        };
        data_chunk passphrase(passphrase_.begin(), passphrase_.end());

        data_chunk key;
        data_chunk iv;
        derive_key(passphrase, salt, key, iv);

        const auto in = Pkcs7_Padding(data_chunk(message.begin(), message.end()));

        data_chunk encrypted = aes256_cbc_encrypt(key, iv, in);

        data_chunk prefix = {0x53, 0x61, 0x6c, 0x74, 0x65, 0x64, 0x5f, 0x5f};

        CONCAT(prefix, salt);
        CONCAT(prefix, encrypted);

        return libbitcoin::encode_base64(prefix);
    }

    std::string decrypt(const std::string &cipher_txt, const std::string &passphrase_)
    {
        data_chunk cipher_data;
        if (false == libbitcoin::decode_base64(cipher_data, cipher_txt)){
            return "base64 cipher text decode error";
        }

        const data_chunk prefix = {0x53, 0x61, 0x6c, 0x74, 0x65, 0x64, 0x5f, 0x5f};
        auto it = std::find_first_of(cipher_data.begin(), cipher_data.end(), prefix.begin(), prefix.end());
        if (it != cipher_data.begin()){
            return "invalid cipher text";
        }

        const data_chunk salt(cipher_data.begin() + 8, cipher_data.begin() + 16);
        const data_chunk encrypted(cipher_data.begin() + 16, cipher_data.end());

        data_chunk passphrase(passphrase_.begin(), passphrase_.end());
        data_chunk key;
        data_chunk iv;
        derive_key(passphrase, salt, key, iv);

        data_chunk plain_data = aes256_cbc_decrypt(key, iv, encrypted);
        const uint8_t padding = plain_data[plain_data.size() - 1];
        if (padding > 0x10) {
            return "invalid cipher text";
        }

        return std::string(plain_data.begin(), plain_data.begin() + plain_data.size() - padding);
    }
}