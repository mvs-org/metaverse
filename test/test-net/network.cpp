/*
 * network.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: jiang
 */




#include <future>
#include <metaverse/network.hpp>
#include <boost/test/unit_test.hpp>

// Send a transaction to a single P2P node.
BOOST_AUTO_TEST_SUITE(network_wiki)

BOOST_AUTO_TEST_CASE(case_network_wiki)
{
    using namespace bc;

#if 0

    // Decode a base16-encoded Bitcoin transaction.
    data_chunk decoded;
    if (argc < 1 || !decode_base16(decoded, argv[0]))
        return -1;

    // Parse the decoded transaction.
    const auto tx = chain::transaction::factory_from_data(decoded);

    // Configure the P2P network session for best performance.
    auto settings = network::settings::mainnet;
    settings.inbound_port = 0;
    settings.host_pool_capacity = 0;
    settings.outbound_connections = 0;
    settings.relay_transactions = false;
    settings.manual_retry_limit = 3;

    // Start a network session.
    network::p2p network(settings);

    // Declare completion signal.
    std::promise<code> complete;

    const auto send_handler = [&complete](code ec)
    {
        complete.set_value(ec);
    };

    const auto connect_handler = [&complete, &tx , &send_handler](
        code ec, network::channel::ptr node)
    {
        if (ec)
            complete.set_value(ec);
        else
            node->send(tx, send_handler);
    };

    // Connect to the one specified host with retry.
    network.connect("localhost", 8333, connect_handler);

    // Wait for completion and return result.
    return complete.get_future().get() ? -1 : 0;

#endif

}

BOOST_AUTO_TEST_SUITE_END()
