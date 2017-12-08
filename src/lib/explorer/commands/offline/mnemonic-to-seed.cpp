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
#include <metaverse/explorer/commands/mnemonic-to-seed.hpp>

#include <iostream>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::config;
using namespace bc::wallet;

console_result mnemonic_to_seed::invoke(std::ostream& output,
    std::ostream& error)
{
    // Bound parameters.
    const dictionary_list& language = get_language_option();
    const auto& passphrase = get_passphrase_option();
    const auto& words = get_words_argument();

    const auto word_count = words.size();

    if ((word_count % bc::wallet::mnemonic_word_multiple) != 0)
    {
        error << BX_EC_MNEMONIC_TO_SEED_LENGTH_INVALID_SENTENCE << std::flush;
        return console_result::failure;
    }

    const auto valid = validate_mnemonic(words, language);

    if (!valid && language.size() == 1)
    {
        // This is fatal because a dictionary was specified explicitly.
        error << BX_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGE << std::flush;
        return console_result::failure;
    }

    if (!valid && language.size() > 1)
    {
        error << BX_EC_MNEMONIC_TO_SEED_INVALID_IN_LANGUAGES << std::flush;
        return console_result::failure;
    }

#ifdef WITH_ICU
    // Any word set divisible by 3 works regardless of language validation.
    const auto seed = decode_mnemonic(words, passphrase);
#else
    if (!passphrase.empty())
    {
        error << BX_EC_MNEMONIC_TO_SEED_PASSPHRASE_UNSUPPORTED << std::flush;
        return console_result::failure;
    }

    // The passphrase requires ICU normalization.
    const auto seed = decode_mnemonic(words);
#endif

    output << base16(seed) << std::flush;
    return console_result::okay;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
