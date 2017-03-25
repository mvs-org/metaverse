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
#include <metaverse/explorer/config/metaverse_output.hpp>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/config/script.hpp>
#include <metaverse/explorer/utility.hpp>

using namespace po;

namespace libbitcoin {
namespace explorer {
namespace config {

metaverse_output::metaverse_output()
  : amount_(0), version_(0), script_(), pay_to_hash_(null_short_hash),
    ephemeral_data_({})
{
}

metaverse_output::metaverse_output(const std::string& tuple)
  : metaverse_output()
{
    std::stringstream(tuple) >> *this;
}

uint64_t metaverse_output::amount() const
{
    return amount_;
}

uint64_t metaverse_output::value() const
{
    return value_;
}

uint8_t metaverse_output::version() const
{
    return version_;
}

const chain::script& metaverse_output::script() const
{
    return script_;
}

const short_hash& metaverse_output::pay_to_hash() const
{
    return pay_to_hash_;
}

const data_chunk& metaverse_output::ephemeral_data() const
{
    return ephemeral_data_;
}

attachment& metaverse_output::attach_data() 
{
    return attach_data_;
}

/*
TARGET:SATOSHI:SEED. 

TARGET is an address (including stealth or pay-to-script-hash) or a Base16 script.  
SATOSHI is the 64 bit spend amount in satoshi. 
SEED is required for stealth outputs and not used otherwise. The same seed should NOT be used for multiple outputs.                       

TARGET:TX-VERSION:BUSINESS-TYPE:XXX:XXX
  0       1             2        3   4
XXX -- content decided by BUSINESS-TYPE

support format :
	 target:1:etp                  :amount(value)  // 4
	 target:1:etp-award     	   :amount(value)  // 4
	 target:1:asset-issue   :symbol:value          // 4
	 target:1:asset-transfer:symbol:amount:value   // 5

test command eg:

encodeattachtx test test -o 1BsM2JbiPMSzGRJHQS2eNewK4SY9MGa4E5:1:asset-transfer:car:9:xxx
encodeattachtx test test -o 1BsM2JbiPMSzGRJHQS2eNewK4SY9MGa4E5:1:asset-issue:car:xxx

*/
std::istream& operator>>(std::istream& input, metaverse_output& argument)
{
    std::string tuple;
    input >> tuple;

    const auto tokens = split(tuple, BX_TX_POINT_DELIMITER);
    if (tokens.size() < 4)
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(tuple));
    }

    uint32_t version;
    deserialize(version, tokens[1], true);
	
    std::string type;
    deserialize(type, tokens[2], true);

	std::string symbol;
    uint64_t amount = 0;
    uint64_t value = 0;
    auto target = tokens.front();
	if(type == "etp") {
		deserialize(value, tokens[3], true);
		argument.attach_data_ = attachment(ETP_TYPE, version, etp(value));
	} else if(type == "etp-award") {
		deserialize(value, tokens[3], true);
		argument.attach_data_ = attachment(ETP_AWARD_TYPE, version, etp(value));
	} else if(type == "asset-issue") {
		deserialize(symbol, tokens[3], true);
		deserialize(value, tokens[4], true);
		auto detail = asset_detail(symbol, 0, 0, "", target, ""); // fill later
		auto ass = asset(ASSET_DETAIL_TYPE, detail);
		argument.attach_data_ = attachment(ASSET_TYPE, version, ass);
	} else if(type == "asset-transfer") {
		deserialize(symbol, tokens[3], true);
		deserialize(amount, tokens[4], true);
		deserialize(value, tokens[5], true);
		auto transfer = asset_transfer(symbol, amount);
		auto ass = asset(ASSET_TRANSFERABLE_TYPE, transfer);
		argument.attach_data_ = attachment(ASSET_TYPE, version, ass);
	} else {
        BOOST_THROW_EXCEPTION(invalid_option_value(tuple));
	}
    if (amount > max_money())
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(tuple));
    }

    argument.amount_ = amount;
    argument.value_ = value;
	log::debug("metaverse_output") << "target=" << target;
	log::debug("metaverse_output") << "version=" << version;
	log::debug("metaverse_output") << "type=" << type;
	log::debug("metaverse_output") << "symbol=" << symbol;
	log::debug("metaverse_output") << "amount=" << amount;
    // Is the target a payment address?
    const wallet::payment_address payment(target);
    if (payment)
    {
        argument.version_ = payment.version();
        argument.pay_to_hash_ = payment.hash();
        return input;
    }
    
    // The target must be a serialized script.
    data_chunk decoded;
    if (!decode_base16(decoded, target))
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(target));
    }

    argument.script_ = script(decoded);
    return input;
}

} // namespace explorer
} // namespace config
} // namespace libbitcoin
