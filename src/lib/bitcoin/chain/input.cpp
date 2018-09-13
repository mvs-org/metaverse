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
#include <metaverse/bitcoin/chain/input.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {



input::input():sequence(0)
{

}

input::input(input&& other)
: input(std::move(other.previous_output), std::move(other.script),other.sequence)
{
}
input::input(const input& other)
:input(other.previous_output, other.script, other.sequence)
{
}

input::input(output_point&& previous_output, chain::script&& script, uint32_t sequence)
: previous_output(std::move(previous_output)), script(std::move(script))
,sequence(sequence)
{
}

input::input(const output_point& previous_output, const chain::script& script, const uint32_t& sequence)
: previous_output(previous_output), script(script),sequence(sequence)
{
}

input& input::operator=(input&& other)
{
    previous_output = std::move(other.previous_output);
    script = std::move(other.script);
    sequence = std::move(other.sequence);
    return *this;
}

input& input::operator=(const input& other)
{
    previous_output = other.previous_output;
    script = other.script;
    sequence = other.sequence;
    return *this;
}


bool input::is_valid() const
{
    return (sequence != 0) ||
        previous_output.is_valid() ||
        script.is_valid();
}

void input::reset()
{
    previous_output.reset();
    script.reset();
    sequence = 0;
}

bool input::from_data_t(reader& source)
{
    reset();

    auto result = previous_output.from_data(source);

    if (result)
    {
        auto mode = script::parse_mode::raw_data_fallback;

        if (previous_output.is_null())
            mode = script::parse_mode::raw_data;

        result = script.from_data(source, true, mode);
    }

    if (result)
    {
        sequence = source.read_4_bytes_little_endian();
        result = source;
    }

    if (!result)
        reset();

    return result;
}

void input::to_data_t(writer& sink) const
{
    previous_output.to_data(sink);
    script.to_data(sink, true);
    sink.write_4_bytes_little_endian(sequence);
}

uint64_t input::serialized_size() const
{
    const auto script_size = script.serialized_size(true);
    return 4 + previous_output.serialized_size() + script_size;
}

std::string input::to_string(uint32_t flags) const
{
    std::ostringstream ss;

    ss << previous_output.to_string() << "\n"
        << "\t" << script.to_string(flags) << "\n"
        << "\tsequence = " << sequence << "\n";

    return ss.str();
}

bool input::is_final() const
{
    return (sequence == max_input_sequence);
}

bool input::is_locked(size_t block_height, uint32_t median_time_past) const
{
    if ((sequence & relative_locktime_disabled) != 0)
        return false;

    // bip68: a minimum block-height constraint over the input's age.
    const auto minimum = (sequence & relative_locktime_mask);
    const auto& prevout = previous_output.metadata;

    if ((sequence & relative_locktime_time_locked) != 0) {
        // Median time past must be monotonically-increasing by block.
        BITCOIN_ASSERT(median_time_past >= prevout.median_time_past);
        const auto age_seconds = median_time_past - prevout.median_time_past;
        return age_seconds < (minimum << relative_locktime_seconds_shift);
    }

    BITCOIN_ASSERT(block_height >= prevout.height);
    const auto age_blocks = block_height - prevout.height;
    return age_blocks < minimum;
}

std::string input::get_script_address() const
{
    auto payment_address = wallet::payment_address::extract(script);
    return payment_address.encoded();
}

} // namspace chain
} // namspace libbitcoin
