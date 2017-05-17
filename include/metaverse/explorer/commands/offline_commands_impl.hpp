/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
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

#pragma once

#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp> 
#include <metaverse/explorer/utility.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace bw = bc::wallet;
namespace bconfig = bc::config;

// -------------------------------------------------------------------
// macro for version
const uint32_t hd_default_secret_version = 76066276;
const uint32_t hd_default_public_version = 76067358;

// -------------------------------------------------------------------
// macro for error message
#define MVSCLI_SEED_BIT_LENGTH_UNSUPPORTED \
        "The seed size is not supported."
#define MVSCLI_EC_MNEMONIC_NEW_INVALID_ENTROPY \
        "The seed length in bytes is not evenly divisible by 32 bits."
#define MVSCLI_EC_MNEMONIC_TO_SEED_LENGTH_INVALID_SENTENCE \
        "The number of words must be divisible by 3."
#define MVSCLI_EC_MNEMONIC_TO_SEED_PASSPHRASE_UNSUPPORTED \
        "The passphrase option requires an ICU build."
#define MVSCLI_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGE \
        "The specified words are not a valid mnemonic in the specified dictionary."
#define MVSCLI_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGES \
        "WARNING: The specified words are not a valid mnemonic in any supported dictionary."
#define MVSCLI_HD_NEW_SHORT_SEED \
        "The seed is less than 128 bits long."
#define MVSCLI_HD_NEW_INVALID_KEY \
        "The seed produced an invalid key."



// -------------------------------------------------------------------
// decalration for functions

data_chunk get_seed(uint16_t bit_length);

bw::word_list get_mnemonic_new(const bw::dictionary_list& language, const data_chunk& entropy);

data_chunk get_mnemonic_to_seed(const bw::dictionary_list& language,            
            const std::string& passphrase,                                              
            const bw::word_list& words);

bw::hd_private get_hd_new(const data_chunk& seed, uint32_t version);




} //namespace commands
} //namespace explorer
} //namespace libbitcoin
