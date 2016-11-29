/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-protocol.
 *
 * libbitcoin-protocol is free software: you can redistribute it and/or
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
#ifdef MVS_VERSION4

#include <bitcoin/protocol/converter.hpp>

#include <string>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace protocol {

/**
 * Copy `binary` data from protobuf's storage format (std::string)
 * to libbitcoin's storage format (hash_digest).
 */
static bool unpack_hash(hash_digest& out, const std::string& in)
{
    if (in.size() != hash_size)
        return false;

    std::copy(in.begin(), in.end(), out.begin());
    return true;
}

static std::string pack_hash(hash_digest in)
{
    return std::string(in.begin(), in.end());
}

bool converter::from_protocol(const point* point, chain::output_point& result)
{
    if (point == nullptr)
        return false;

    result.index = point->index();
    return unpack_hash(result.hash, point->hash());
}

bool converter::from_protocol(const std::shared_ptr<point> point,
    chain::output_point& result)
{
    return from_protocol(point.get(), result);
}

bool converter::from_protocol(const tx_input* input, chain::input& result)
{
    if (input == nullptr)
        return false;

    chain::output_point previous;
    if (!input->has_previous_output() ||
        !from_protocol(&(input->previous_output()), previous) ||
        !input->has_script())
        return false;

    result.previous_output = previous;
    result.sequence = input->sequence();
    const auto script_text = input->script();
    const data_chunk data(script_text.begin(), script_text.end());

    // protocol question - is the data encoding of the script to be
    // prefixed with operation count?
    return result.script.from_data(data, false,
        chain::script::parse_mode::raw_data_fallback);
}

bool converter::from_protocol(const std::shared_ptr<tx_input> input,
    chain::input& result)
{
    return from_protocol(input.get(), result);
}

bool converter::from_protocol(const tx_output* output, chain::output& result)
{
    if (output == nullptr || !output->has_script())
        return false;

    result.value = output->value();
    const auto script_text = output->script();
    const data_chunk data(script_text.begin(), script_text.end());

    // protocol question - is the data encoding of the script to be
    // prefixed with operation count?
    return result.script.from_data(data, false,
        chain::script::parse_mode::raw_data_fallback);
}

bool converter::from_protocol(const std::shared_ptr<tx_output> output,
    chain::output& result)
{
    return from_protocol(output.get(), result);
}

bool converter::from_protocol(const tx* transaction,
    chain::transaction& result)
{
    if (transaction == nullptr)
        return false;

    auto success = true;
    result.version = transaction->version();
    result.locktime = transaction->locktime();

    for (auto input: transaction->inputs())
    {
        chain::input bitcoin_input;

        if (!from_protocol(&input, bitcoin_input))
        {
            success = false;
            break;
        }

        result.inputs.push_back(bitcoin_input);
    }

    if (success)
    {
        for (auto output: transaction->outputs())
        {
            chain::output bitcoin_output;

            if (!from_protocol(&output, bitcoin_output))
            {
                success = false;
                break;
            }

            result.outputs.push_back(bitcoin_output);
        }
    }

    if (!success)
    {
        result.version = 0;
        result.locktime = 0;
        result.inputs.clear();
        result.outputs.clear();
    }

    return success;
}

bool converter::from_protocol(const std::shared_ptr<tx> transaction,
    chain::transaction& result)
{
    return from_protocol(transaction.get(), result);
}

bool converter::from_protocol(const block_header* header,
    chain::header& result)
{
    if (header == nullptr)
        return false;

    result.version = header->version();
    result.timestamp = header->timestamp();
    result.bits = header->bits();
    result.nonce = header->nonce();
    result.transaction_count = header->tx_count();
    return unpack_hash(result.merkle, header->merkle_root()) &&
        unpack_hash(result.previous_block_hash, header->previous_block_hash());
}

bool converter::from_protocol(const std::shared_ptr<block_header> header,
    chain::header& result)
{
    return from_protocol(header.get(), result);
}

bool converter::from_protocol(const protocol::block* block,
    chain::block& result)
{
    if (block == nullptr || !from_protocol(&(block->header()), result.header))
        return false;

    for (auto tx: block->transactions())
    {
        chain::transaction bitcoin_tx;

        if (!from_protocol(&tx, bitcoin_tx))
        {
            result.transactions.clear();
            return false;;
        }

        result.transactions.push_back(bitcoin_tx);
    }

    return true;
}

bool converter::from_protocol(const std::shared_ptr<block> block,
    chain::block& result)
{
    return from_protocol(block.get(), result);
}

bool converter::to_protocol(const chain::output_point& point,
    protocol::point& result)
{
    result.set_hash(pack_hash(point.hash));
    result.set_index(point.index);
    return true;
}

point* converter::to_protocol(const chain::output_point& point)
{
    std::unique_ptr<protocol::point> result(new protocol::point());

    if (!to_protocol(point, *(result.get())))
        result.reset();

    return result.release();
}

bool converter::to_protocol(const chain::input& input, tx_input& result)
{
    const auto& previous_output = input.previous_output;
    result.set_allocated_previous_output(to_protocol(previous_output));

    // protocol question - is the data encoding of the script to be prefixed
    // with operation count?
    const auto script_data = input.script.to_data(false);
    result.set_script(std::string(script_data.begin(), script_data.end()));
    result.set_sequence(input.sequence);
    return true;
}

tx_input* converter::to_protocol(const chain::input& input)
{
    std::unique_ptr<tx_input> result(new tx_input());

    if (!to_protocol(input, *(result.get())))
        result.reset();

    return result.release();
}

bool converter::to_protocol(const chain::output& output, tx_output& result)
{
    result.set_value(output.value);

    // protocol question - is the data encoding of the script to be prefixed
    // with operation count?
    const auto script_data = output.script.to_data(false);
    result.set_script({ script_data.begin(), script_data.end() });
    return true;
}

tx_output* converter::to_protocol(const chain::output& output)
{
    std::unique_ptr<tx_output> result(new tx_output());

    if (!to_protocol(output, *(result.get())))
        result.reset();

    return result.release();
}

bool converter::to_protocol(const chain::transaction& transaction, tx& result)
{
    result.set_version(transaction.version);
    result.set_locktime(transaction.locktime);
    auto repeated_inputs = result.mutable_inputs();

    auto success = true;

    for (const auto& input: transaction.inputs)
    {
        if (!to_protocol(input, *(repeated_inputs->Add())))
        {
            success = false;
            break;
        }
    }

    auto repeated_outputs = result.mutable_outputs();

    if (success)
    {
        for (const auto& output: transaction.outputs)
        {
            if (!to_protocol(output, *(repeated_outputs->Add())))
            {
                success = false;
                break;
            }
        }
    }

    if (!success)
    {
        result.clear_version();
        result.clear_locktime();
        result.clear_inputs();
        result.clear_outputs();
    }

    return success;
}

tx* converter::to_protocol(const chain::transaction& transaction)
{
    std::unique_ptr<tx> result(new tx());

    if (!to_protocol(transaction, *(result.get())))
        result.reset();

    return result.release();
}

bool converter::to_protocol(const chain::header& header, block_header& result)
{
    result.set_version(header.version);
    result.set_timestamp(header.timestamp);
    result.set_bits(header.bits);
    result.set_nonce(header.nonce);
    result.set_merkle_root(pack_hash(header.merkle));
    result.set_previous_block_hash(pack_hash(header.previous_block_hash));
    result.set_tx_count(header.transaction_count);
    return true;
}

block_header* converter::to_protocol(const chain::header& header)
{
    std::unique_ptr<block_header> result(new block_header());

    if (!to_protocol(header, *(result.get())))
        result.reset();

    return result.release();
}

bool converter::to_protocol(const chain::block& block, protocol::block& result)
{
    result.set_allocated_header(to_protocol(block.header));
    if (!result.has_header())
        return false;

    auto repeated_transactions = result.mutable_transactions();

    for (const auto& transaction: block.transactions)
    {
        if (!to_protocol(transaction, *(repeated_transactions->Add())))
        {
            result.clear_header();
            result.clear_transactions();
            return false;
        }
    }

    return true;
}

protocol::block* converter::to_protocol(const chain::block& block)
{
    std::unique_ptr<protocol::block> result(new protocol::block());

    if (!to_protocol(block, *(result.get())))
        result.reset();

    return result.release();
}

}
}

#endif
