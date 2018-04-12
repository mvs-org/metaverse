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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_WALLET_DICTIONARY_HPP
#define MVS_WALLET_DICTIONARY_HPP

#include <array>
#include <vector>
#include <metaverse/bitcoin/compat.hpp>

namespace libbitcoin {
namespace wallet {

/**
 * A valid mnemonic dictionary has exactly this many words.
 */
static BC_CONSTEXPR size_t dictionary_size = 2048;

/**
 * A dictionary for creating mnemonics.
 * The bip39 spec calls this a "wordlist".
 * This is a POD type, which means the compiler can write it directly
 * to static memory with no run-time overhead.
 */
typedef std::array<const char*, dictionary_size> dictionary;

/**
 * A collection of candidate dictionaries for mnemonic validation.
 */
typedef std::vector<const dictionary*> dictionary_list;

namespace language
{
    // Individual built-in languages:
    extern const dictionary en;
    extern const dictionary es;
    extern const dictionary ja;
    extern const dictionary zh_Hans;
    extern const dictionary zh_Hant;

    // All built-in languages:
    extern const dictionary_list all;
}

namespace symbol
{
    typedef std::array<const char*, 65> dictionary2;
    // built in ban dict (upper case):
    extern const dictionary2 ban_list; //S level
    bool is_sensitive(const std::string& symbol);

    // All built-in ban list:
}

} // namespace wallet
} // namespace libbitcoin

#endif
