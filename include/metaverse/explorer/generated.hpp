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
#ifndef BX_GENERATED_HPP
#define BX_GENERATED_HPP

#include <functional>
#include <memory>
#include <string>
#include <metaverse/explorer/command.hpp>
#include <metaverse/explorer/commands/address-decode.hpp>
#include <metaverse/explorer/commands/address-embed.hpp>
#include <metaverse/explorer/commands/address-encode.hpp>
#include <metaverse/explorer/commands/base16-decode.hpp>
#include <metaverse/explorer/commands/base16-encode.hpp>
#include <metaverse/explorer/commands/base58-decode.hpp>
#include <metaverse/explorer/commands/base58-encode.hpp>
#include <metaverse/explorer/commands/base58check-decode.hpp>
#include <metaverse/explorer/commands/base58check-encode.hpp>
#include <metaverse/explorer/commands/base64-decode.hpp>
#include <metaverse/explorer/commands/base64-encode.hpp>
#include <metaverse/explorer/commands/bitcoin160.hpp>
#include <metaverse/explorer/commands/bitcoin256.hpp>
#include <metaverse/explorer/commands/btc-to-satoshi.hpp>
#include <metaverse/explorer/commands/cert-new.hpp>
#include <metaverse/explorer/commands/cert-public.hpp>
#include <metaverse/explorer/commands/ec-add.hpp>
#include <metaverse/explorer/commands/ec-add-secrets.hpp>
#include <metaverse/explorer/commands/ec-multiply.hpp>
#include <metaverse/explorer/commands/ec-multiply-secrets.hpp>
#include <metaverse/explorer/commands/ec-new.hpp>
#include <metaverse/explorer/commands/ec-to-address.hpp>
#include <metaverse/explorer/commands/ec-to-ek.hpp>
#include <metaverse/explorer/commands/ec-to-public.hpp>
#include <metaverse/explorer/commands/ec-to-wif.hpp>
#include <metaverse/explorer/commands/ek-address.hpp>
#include <metaverse/explorer/commands/ek-new.hpp>
#include <metaverse/explorer/commands/ek-public.hpp>
#include <metaverse/explorer/commands/ek-public-to-address.hpp>
#include <metaverse/explorer/commands/ek-public-to-ec.hpp>
#include <metaverse/explorer/commands/ek-to-address.hpp>
#include <metaverse/explorer/commands/ek-to-ec.hpp>
#include <metaverse/explorer/commands/fetch-balance.hpp>
#include <metaverse/explorer/commands/fetch-header.hpp>
#include <metaverse/explorer/commands/fetch-height.hpp>
#include <metaverse/explorer/commands/fetch-history.hpp>
#include <metaverse/explorer/commands/fetch-public-key.hpp>
#include <metaverse/explorer/commands/fetch-stealth.hpp>
#include <metaverse/explorer/commands/fetch-tx.hpp>
#include <metaverse/explorer/commands/fetch-tx-index.hpp>
#include <metaverse/explorer/commands/fetch-utxo.hpp>
#include <metaverse/explorer/commands/hd-new.hpp>
#include <metaverse/explorer/commands/hd-private.hpp>
#include <metaverse/explorer/commands/hd-public.hpp>
#include <metaverse/explorer/commands/hd-to-address.hpp>
#include <metaverse/explorer/commands/hd-to-ec.hpp>
#include <metaverse/explorer/commands/hd-to-public.hpp>
#include <metaverse/explorer/commands/hd-to-wif.hpp>
#include <metaverse/explorer/commands/help.hpp>
#include <metaverse/explorer/commands/input-set.hpp>
#include <metaverse/explorer/commands/input-sign.hpp>
#include <metaverse/explorer/commands/input-validate.hpp>
#include <metaverse/explorer/commands/message-sign.hpp>
#include <metaverse/explorer/commands/message-validate.hpp>
#include <metaverse/explorer/commands/mnemonic-decode.hpp>
#include <metaverse/explorer/commands/mnemonic-encode.hpp>
#include <metaverse/explorer/commands/mnemonic-new.hpp>
#include <metaverse/explorer/commands/mnemonic-to-seed.hpp>
#include <metaverse/explorer/commands/qrcode.hpp>
#include <metaverse/explorer/commands/ripemd160.hpp>
#include <metaverse/explorer/commands/satoshi-to-btc.hpp>
#include <metaverse/explorer/commands/script-decode.hpp>
#include <metaverse/explorer/commands/script-encode.hpp>
#include <metaverse/explorer/commands/script-to-address.hpp>
#include <metaverse/explorer/commands/seed.hpp>
#include <metaverse/explorer/commands/send-tx.hpp>
#include <metaverse/explorer/commands/send-tx-node.hpp>
#include <metaverse/explorer/commands/send-tx-p2p.hpp>
#include <metaverse/explorer/commands/settings.hpp>
#include <metaverse/explorer/commands/sha160.hpp>
#include <metaverse/explorer/commands/sha256.hpp>
#include <metaverse/explorer/commands/sha512.hpp>
#include <metaverse/explorer/commands/stealth-decode.hpp>
#include <metaverse/explorer/commands/stealth-encode.hpp>
#include <metaverse/explorer/commands/stealth-public.hpp>
#include <metaverse/explorer/commands/stealth-secret.hpp>
#include <metaverse/explorer/commands/stealth-shared.hpp>
#include <metaverse/explorer/commands/token-new.hpp>
#include <metaverse/explorer/commands/tx-decode.hpp>
#include <metaverse/explorer/commands/tx-encode.hpp>
#include <metaverse/explorer/commands/tx-sign.hpp>
#include <metaverse/explorer/commands/uri-decode.hpp>
#include <metaverse/explorer/commands/uri-encode.hpp>
#include <metaverse/explorer/commands/validate-tx.hpp>
#include <metaverse/explorer/commands/watch-address.hpp>
#include <metaverse/explorer/commands/watch-tx.hpp>
#include <metaverse/explorer/commands/wif-to-ec.hpp>
#include <metaverse/explorer/commands/wif-to-public.hpp>
#include <metaverse/explorer/commands/wrap-decode.hpp>
#include <metaverse/explorer/commands/wrap-encode.hpp>
#include <metaverse/explorer/define.hpp>

/********* GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY **********/

namespace libbitcoin {
namespace explorer {

/**
 * Various shared localizable strings.
 */
#define BX_COMMANDS_HEADER \
    "Info: The commands are:"
#define BX_COMMANDS_HOME_PAGE \
    "MVS home page:"
#define BX_COMMAND_USAGE \
    "Usage: help COMMAND"
#define BX_CONFIG_DESCRIPTION \
    "The path to the configuration settings file."
#define BX_CONNECTION_FAILURE \
    "{\"error\":\"Could not connect to server: %1%\"}"
#define BX_DEPRECATED_COMMAND \
    "{\"error\":\"The '%1%' command has been replaced by '%2%'.\"}"
#define BX_HELP_DESCRIPTION \
    "Get a description and instructions for this command."
#define BX_INVALID_COMMAND \
    "{\"error\":\"'%1%' is not a command. Enter 'help' for a list of commands.\"}"
#define BX_INVALID_PARAMETER \
    "{\"error\": \"%1%\"}"
#define BX_PRINTER_ARGUMENT_TABLE_HEADER \
    "Arguments (positional):"
#define BX_PRINTER_DESCRIPTION_FORMAT \
    "Info: %1%"
#define BX_PRINTER_OPTION_TABLE_HEADER \
    "Options (named):"
#define BX_PRINTER_USAGE_FORMAT \
    "Usage: %1% %2% %3%"
#define BX_PRINTER_VALUE_TEXT \
    "VALUE"
#define BX_VERSION_MESSAGE \
    "Version: %1%"

/**
 * Invoke a specified function on all commands.
 * @param[in]  func  The function to invoke on all commands.
 */
void broadcast(const std::function<void(std::shared_ptr<command>)> func, std::ostream& os);

/**
 * Find the command identified by the specified symbolic command name.
 * @param[in]  symbol  The symbolic command name.
 * @return             An instance of the command or nullptr if not found.
 */
std::shared_ptr<command> find(const std::string& symbol);

/**
 * Find the new name of the formerly-named command.
 * @param[in]  former  The former symbolic command name.
 * @return             The current name of the formerly-named command.
 */
std::string formerly(const std::string& former);

} // namespace explorer
} // namespace libbitcoin

#endif
