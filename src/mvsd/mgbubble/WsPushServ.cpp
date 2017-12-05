/*
* Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS).
* Copyright (C) 2013, 2016 Swirly Cloud Limited.
*
* This program is free software; you can redistribute it and/or modify it under the terms of the
* GNU General Public License as published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
* even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program; if
* not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include <thread>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/mgbubble/WsPushServ.hpp>
#include <metaverse/server/server_node.hpp>

namespace mgbubble {
    constexpr auto EV_VERSION    = "version";
    constexpr auto EV_SUBSCRIBE  = "subscribe";
    constexpr auto EV_SUBSCRIBED = "subscribed";
    constexpr auto EV_PUBLISH    = "publish";
    constexpr auto EV_REQUEST    = "request";
    constexpr auto EV_RESPONSE   = "response";
    constexpr auto EV_ERROR      = "error";

    constexpr auto CH_BLOCK       = "block";
    constexpr auto CH_TRANSACTION = "tx";
}
namespace mgbubble {
using namespace bc;
using namespace pt;
using namespace libbitcoin;

void WsPushServ::run() {
    log::info(NAME) << "Websocket Service listen on " << node_.server_settings().websocket_listen;

    node_.subscribe_stop([this](const libbitcoin::code& ec) { stop(); });

    node_.subscribe_transaction_pool(
        std::bind(&WsPushServ::handle_transaction_pool,
            this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    
    node_.subscribe_blockchain(
        std::bind(&WsPushServ::handle_blockchain_reorganization,
            this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    base::run();
}

bool WsPushServ::start()
{
    if (node_.server_settings().websocket_service_enabled == false)
        return true;
    if (!attach_notify())
        return false;
    return base::start();
}

void WsPushServ::spawn_to_mongoose(const std::function<void(uint64_t)>&& handler)
{
    auto msg = std::make_shared<WsEvent>(std::move(handler));
    struct mg_event ev { msg->hook() };
    if (!notify(ev))
        msg->unhook();
}

bool WsPushServ::handle_transaction_pool(const code& ec, const index_list&, message::transaction_message::ptr tx)
{
    if (stopped())
        return false;
    if (ec == (code)error::mock || ec == (code)error::service_stopped)
        return true;
    if (ec)
    {
        log::debug(NAME) << "Failure handling new transaction: " << ec.message();
        return true;
    }

    notify_transaction(0, null_hash, *tx);
    return true;
}

bool WsPushServ::handle_blockchain_reorganization(const code& ec, uint64_t fork_point, const block_list& new_blocks, const block_list&)
{
    if (stopped())
        return false;
    if (ec == (code)error::mock || ec == (code)error::service_stopped)
        return true;
    if (ec)
    {
        log::debug(NAME) << "Failure handling new block: " << ec.message();
        return true;
    }

    const auto fork_point32 = static_cast<uint32_t>(fork_point);

    notify_blocks(fork_point32, new_blocks);
    return true;
}

void WsPushServ::notify_blocks(uint32_t fork_point, const block_list& blocks)
{
    if (stopped())
        return;
    
    auto height = fork_point;

    for (const auto block : blocks)
        notify_block(height++, block);
}

void WsPushServ::notify_block(uint32_t height, const block::ptr block)
{
    if (stopped())
        return;

    const auto block_hash = block->header.hash();

    for (const auto& tx : block->transactions)
    {
        const auto tx_hash = tx.hash();

        notify_transaction(height, block_hash, tx);
    }
}

void WsPushServ::notify_transaction(uint32_t height, const hash_digest& block_hash, const transaction& tx)
{
    if (stopped() || tx.outputs.empty())
        return;

    std::stringstream ss;
    ptree root;
    root.put("event", EV_PUBLISH);
    root.put("channel", CH_TRANSACTION);
    root.add_child("result", explorer::config::prop_list(tx, height, true));
    write_json(ss, root);
    
    log::info(NAME) << " ******** notify_transaction: height [" << height << "]  ******** ";
    
    spawn_to_mongoose([height](uint64_t id) {
        log::info(NAME) << " ******** on message: [" << height << " " << id << "]  ******** ";
    });
}

void WsPushServ::send_bad_request(struct mg_connection& nc)
{
    std::stringstream ss;
    ptree root;
    root.put("event", EV_ERROR);
    root.put("msg", "bad request");
    write_json(ss, root);
    auto&& tmp = ss.str();
    send_frame(nc, tmp.c_str(), tmp.size());
}

void WsPushServ::on_ws_handshake_done_handler(struct mg_connection& nc)
{
    std::string version("{\"event\": \"version\", " "\"result\": \"" MVS_VERSION "\"}");
    send_frame(nc, version);
}

void WsPushServ::on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg)
{
    std::istringstream iss;
    iss.str(std::string((const char*)msg.data, msg.size));
    ptree parser;
    try {
        read_json(iss, parser);
        auto event = parser.get<std::string>("event");
        auto channel = parser.get<std::string>("channel");
    }
    catch (std::exception& e) {
        log::info("TEST") << "on on_ws_frame_handler: " << e.what();
    }
    send_bad_request(nc);
    spawn_to_mongoose([](size_t id) {
        log::info("TEST") << "on on_ws_frame_handler: " << id;
    });
}

void WsPushServ::on_close_handler(struct mg_connection& nc)
{
    if (is_websocket(nc))
    {
    }
}

void WsPushServ::on_broadcast(struct mg_connection& nc, const char* ev_data)
{
    if (is_listen_socket(nc) || is_notify_socket(nc))
        return;

    send_frame(nc, ev_data, strlen(ev_data));
}

void WsPushServ::on_send_handler(struct mg_connection& nc, int bytes_transfered)
{
}

void WsPushServ::on_notify_handler(struct mg_connection& nc, struct mg_event& ev)
{
    static uint64_t api_call_counter = 0;

    if (ev.data == nullptr)
        return;

    auto& msg = *(WsEvent*)ev.data;
    msg(++api_call_counter);
}

}