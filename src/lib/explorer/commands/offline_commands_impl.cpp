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

#include <metaverse/explorer/commands/offline_commands_impl.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

//seed
data_chunk get_seed(uint16_t bit_length = 256u)
{
    // These are soft requirements for security and rationality.
    // We use bit vs. byte length input as the more familiar convention.
    if (bit_length < minimum_seed_size * byte_bits ||
        bit_length % byte_bits != 0)
    {
		// chenhao exception
		throw mvscli_seed_size_exception{MVSCLI_SEED_BIT_LENGTH_UNSUPPORTED};
    }

    return new_seed(bit_length);
}

//mnemonic-new
bw::word_list get_mnemonic_new(const bw::dictionary_list& language, const data_chunk& entropy)
{
    const auto entropy_size = entropy.size();

    if ((entropy_size % bw::mnemonic_seed_multiple) != 0)
    {
		// chenhao exception
        throw mvscli_seed_length_exception{MVSCLI_EC_MNEMONIC_NEW_INVALID_ENTROPY};
    }

    // If 'any' default to first ('en'), otherwise the one specified.
    const auto dictionary = language.front();
    return bw::create_mnemonic(entropy, *dictionary);
}


//mnemonic-to-seed
data_chunk get_mnemonic_to_seed(const bw::dictionary_list& language, 
    const std::string& passphrase, 
    const bw::word_list& words)
{
    const auto word_count = words.size();

    if ((word_count % bw::mnemonic_word_multiple) != 0)
    {
		// chenhao exception
        throw mvscli_mnemonicwords_amount_exception{MVSCLI_EC_MNEMONIC_TO_SEED_LENGTH_INVALID_SENTENCE};
    }

    const auto valid = bw::validate_mnemonic(words, language);

    if (!valid && language.size() == 1)
    {
        // This is fatal because a dictionary was specified explicitly.
		// chenhao exception
        throw mvscli_mnemonicwords_content_exception{MVSCLI_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGE};
    }

	// chenhao exception
    if (!valid && language.size() > 1)
        throw mvscli_mnemonicwords_content_exception{MVSCLI_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGES};

#ifdef WITH_ICU
    // Any word set divisible by 3 works regardless of language validation.
    const auto seed = bw::decode_mnemonic(words, passphrase);
#else
    if (!passphrase.empty())
    {
	    // chenhao exception
        throw mvscli_mnemonicwords_content_exception{MVSCLI_EC_MNEMONIC_TO_SEED_PASSPHRASE_UNSUPPORTED};
    }

    // The passphrase requires ICU normalization.
    const auto seed = bw::decode_mnemonic(words);
#endif

    const data_chunk& dc_seed = bc::config::base16(seed);
    return dc_seed;
}

//hd-new
bw::hd_private get_hd_new(const data_chunk& seed, uint32_t version = hd_default_secret_version)
{
    if (seed.size() < minimum_seed_size)
    {
	    // chenhao exception
        throw mvscli_hd_length_exception{MVSCLI_HD_NEW_SHORT_SEED};
    }

    // We require the private version, but public is unused here.
    const auto prefixes = bc::wallet::hd_private::to_prefixes(version, 0);
    const bc::wallet::hd_private private_key(seed, prefixes);

    if (!private_key)
    {
	    // chenhao exception
        throw mvscli_hd_key_exception{MVSCLI_HD_NEW_INVALID_KEY};
    }

    return private_key;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
