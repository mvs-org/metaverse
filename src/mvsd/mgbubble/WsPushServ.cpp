/*
* Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS).
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
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/mgbubble/WsPushServ.hpp>
#include <metaverse/server/server_node.hpp>

namespace mgbubble {
constexpr auto EV_VERSION     = "version";
constexpr auto EV_SUBSCRIBE   = "subscribe";
constexpr auto EV_UNSUBSCRIBE = "unsubscribe";
constexpr auto EV_SUBSCRIBED  = "subscribed";
constexpr auto EV_UNSUBSCRIBED = "unsubscribed";
constexpr auto EV_PUBLISH     = "publish";
constexpr auto EV_REQUEST     = "request";
constexpr auto EV_RESPONSE    = "response";
constexpr auto EV_MG_ERROR    = "error";
constexpr auto EV_INFO        = "info";
constexpr auto EV_PING        = "ping";

constexpr auto CH_BLOCK       = "block";
constexpr auto CH_HEIGHT      = "height";
constexpr auto CH_TRANSACTION = "tx";
constexpr auto CH_ALL         = "all";

constexpr int  JSON_FORMAT_VERSION = 3;
}

namespace mgbubble {
using namespace bc;
using namespace bc::chain;

explorer::config::json_helper get_json_helper()
{
    return explorer::config::json_helper(JSON_FORMAT_VERSION);
}

void WsPushServ::run() {
    using namespace std::placeholders;
    log::info(NAME) << "Websocket Service listen on " << node_.server_settings().websocket_listen;

    node_.subscribe_stop([this](const libbitcoin::code & ec) { stop(); });

    node_.subscribe_transaction_pool(
        std::bind(&WsPushServ::handle_transaction_pool,
                  this, _1, _2, _3));

    node_.subscribe_blockchain(
        std::bind(&WsPushServ::handle_blockchain_reorganization,
                  this, _1, _2, _3, _4));

    base::run();

    log::info(NAME) << "Websocket Service Stopped.";
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
    if (ec.value() == error::mock || ec.value() == error::service_stopped)
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
    if (ec.value() == error::mock || ec.value() == error::service_stopped)
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

    //auto height = fork_point;

    for (const auto block : blocks)
        notify_block(block.get()->header.number, block);
}

void WsPushServ::notify_block(uint32_t height, const block::ptr block)
{
    if (stopped())
        return;

    notify_block_impl(height, block);

    const auto block_hash = block->header.hash();
    for (const auto& tx : block->transactions) {
        const auto tx_hash = tx.hash();

        notify_transaction(height, block_hash, tx);
    }
}

void WsPushServ::notify_block_impl(uint32_t height, const bc::chain::block::ptr block)
{
    connection_string_map subscribers;
    {
        std::lock_guard<std::mutex> guard(block_subscribers_lock_);
        if (block_subscribers_.size() == 0) {
            return;
        }

        for (auto& con : block_subscribers_) {
            if (!con.first.expired()) {
                subscribers.insert(con);
            }
        }

        if (subscribers.size() != block_subscribers_.size()) {
            block_subscribers_ = subscribers;
        }
    }

    std::vector<std::weak_ptr<mg_connection>> notify_block_cons;
    std::vector<std::weak_ptr<mg_connection>> notify_height_cons;
    for (auto& sub : subscribers) {
        auto& params = sub.second;
        if (params.end() != std::find(params.begin(), params.end(), CH_HEIGHT)) {
            notify_height_cons.push_back(sub.first);
        }

        if (params.end() != std::find(params.begin(), params.end(), CH_BLOCK)) {
            notify_block_cons.push_back(sub.first);
        }
    }

    if (notify_block_cons.size() > 0) {
        Json::Value root;
        root["event"] = EV_PUBLISH;
        root["channel"] = CH_BLOCK;
        root["result"] = get_json_helper().prop_tree(*block, true, true);

        // log::info(NAME) << " ******** notify_block: height [" << height << "]  ******** ";

        do_notify(notify_block_cons, root);
    }

    if (notify_height_cons.size() > 0) {
        Json::Value root;
        root["event"] = EV_PUBLISH;
        root["channel"] = CH_HEIGHT;
        root["result"] = height;

        // log::info(NAME) << " ******** notify_height: height [" << height << "]  ******** ";

        do_notify(notify_height_cons, root);
    }
}

void WsPushServ::do_notify(
    const std::vector<std::weak_ptr<mg_connection>>& notify_cons,
    Json::Value& root,
    std::shared_ptr<connection_string_map> topic_map)
{
    auto orignal_rep = root.toStyledString();

    for (auto& con : notify_cons)
    {
        auto shared_con = con.lock();
        if (!shared_con) {
            continue;
        }

        std::string rep;
        if (topic_map != nullptr) {
            auto iter = topic_map->find(con);
            if (iter != topic_map->end()) {
                auto& topics = iter->second;
                if (topics.end() != std::find(topics.begin(), topics.end(), CH_ALL)) {
                    root["topic"] = CH_ALL;
                }
                else if (topics.size() == 1) {
                    root["topic"] = topics[0];
                }
                else {
                    Json::Value value;
                    for (auto& topic : topics) {
                        value.append(topic);
                    }
                    if (value.isNull())
                        value.resize(0);

                    root["topic"] = value;
                }

                rep = root.toStyledString();
            }
        }

        if (rep.empty()) {
            rep = orignal_rep;
        }

        spawn_to_mongoose([this, shared_con, rep](uint64_t id) {
            size_t active_connections = 0;
            auto* mgr = &this->mg_mgr();
            auto* notify_nc = shared_con.get();
            for (auto* nc = mg_next(mgr, NULL); nc != NULL; nc = mg_next(mgr, nc)) {
                if (!is_websocket(*nc) || is_listen_socket(*nc) || is_notify_socket(*nc))
                    continue;
                ++active_connections;
                if (notify_nc == nc) {
                    send_frame(*nc, rep);
                }
            }

            if (active_connections != map_connections_.size()) {
                refresh_connections();
            }
        });
    }
}

void WsPushServ::notify_transaction(uint32_t height, const hash_digest& block_hash, const transaction& tx)
{
    if (stopped() || tx.outputs.empty()) {
        return;
    }

    connection_string_map subscribers;
    {
        std::lock_guard<std::mutex> guard(subscribers_lock_);
        if (subscribers_.size() == 0) {
            return;
        }

        for (auto& con : subscribers_) {
            if (!con.first.expired()) {
                subscribers.insert(con);
            }
        }

        if (subscribers.size() != subscribers_.size()) {
            subscribers_ = subscribers;
        }
    }

    /* ---------- may has subscribers ---------- */

    string_vector tx_addrs;
    for (const auto& input : tx.inputs) {
        const auto address = wallet::payment_address::extract(input.script);
        if (address) {
            auto addr_hash = address.encoded();
            if (tx_addrs.end() == std::find(tx_addrs.begin(), tx_addrs.end(), addr_hash)) {
                tx_addrs.push_back(addr_hash);
            }
        }
    }

    for (const auto& output : tx.outputs) {
        const auto address = wallet::payment_address::extract(output.script);
        if (address) {
            auto addr_hash = address.encoded();
            if (tx_addrs.end() == std::find(tx_addrs.begin(), tx_addrs.end(), addr_hash)) {
                tx_addrs.push_back(addr_hash);
            }
        }
    }

    std::shared_ptr<connection_string_map> topic_map = std::make_shared<connection_string_map>();

    std::vector<std::weak_ptr<mg_connection>> notify_cons;
    for (auto& sub : subscribers)
    {
        string_vector topics;
        auto& sub_addrs = sub.second;
        if (sub_addrs.empty()) {
            topics.push_back(CH_ALL);
        }
        else {
            for (auto& addr_hash : sub_addrs) {
                if (tx_addrs.end() != std::find(tx_addrs.begin(), tx_addrs.end(), addr_hash)) {
                    topics.push_back(addr_hash);
                }
            }
        }

        if (!topics.empty()) {
            topic_map->insert({sub.first, topics});
            notify_cons.push_back(sub.first);
        }
    }

    if (notify_cons.size() == 0) {
        return;
    }

    // log::info(NAME) << " ******** notify_transaction: height [" << height << "]  ******** ";

    Json::Value root;
    root["event"] = EV_PUBLISH;
    root["channel"] = CH_TRANSACTION;
    root["result"] = get_json_helper().prop_list(tx, height, true);

    do_notify(notify_cons, root, topic_map);
}

void WsPushServ::send_bad_response(struct mg_connection& nc, const char* message, int code, Json::Value data)
{
    Json::Value root;
    Json::Value result;
    result["code"] = code;
    result["message"] = message ? message : "bad request";
    if (!data.isNull())
        result["data"] = data;
    root["event"]  = EV_MG_ERROR;
    root["result"] = result;

    auto&& tmp = root.toStyledString();
    send_frame(nc, tmp.c_str(), tmp.size());
}

void WsPushServ::send_response(struct mg_connection& nc, const std::string& event, const std::string& channel, Json::Value data)
{
    Json::Value root;
    root["event"] = event;
    if (!channel.empty()) {
        root["channel"] = channel;
    }

    if (!data.isNull()) {
        root["result"] = data;
    }

    auto&& tmp = root.toStyledString();
    send_frame(nc, tmp.c_str(), tmp.size());
}

void WsPushServ::refresh_connections()
{
    auto* mgr = &mg_mgr();
    std::unordered_map<void*, std::shared_ptr<mg_connection>> swap;
    for (auto* nc = mg_next(mgr, NULL); nc != NULL; nc = mg_next(mgr, nc)) {
        if (!is_websocket(*nc) || is_listen_socket(*nc) || is_notify_socket(*nc))
            continue;
        std::shared_ptr<struct mg_connection> con(nc, [](struct mg_connection * ptr) { (void)(ptr); });
        swap.emplace(&nc, con);
    }
    map_connections_.swap(swap);
}

void WsPushServ::on_ws_handshake_done_handler(struct mg_connection& nc)
{
    std::shared_ptr<struct mg_connection> con(&nc, [](struct mg_connection * ptr) { (void)(ptr); });
    map_connections_.emplace(&nc, con);

    std::string version("{\"event\": \"version\", " "\"result\": \"" MVS_VERSION "\"}");
    send_frame(nc, version);

    std::stringstream ss;
    Json::Value root;
    Json::Value connections;
    connections["connections"] = static_cast<uint64_t>(map_connections_.size());
    root["event"] = EV_INFO;
    root["result"] = connections;

    auto&& tmp = root.toStyledString();
    send_frame(nc, tmp);
}

void WsPushServ::on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg)
{
    Json::Reader reader;
    Json::Value root;
    if (node_.is_blockchain_sync()) {
        send_bad_response(nc, "under blockchain synchronizing", 1000002);
        return;
    }

    try {
        const char* begin = (const char*)msg.data;
        const char* end = begin + msg.size;
        if (!reader.parse(begin, end, root)
                || !root.isObject()
                || !root["event"].isString()) {
            std::stringstream ss;
            ss << "parse request error, "
               << reader.getFormattedErrorMessages();
            throw std::runtime_error(ss.str());
            return;
        }

        std::string channel;
        auto event = root["event"].asString();
        if (event == EV_SUBSCRIBE || event == EV_UNSUBSCRIBE) {
            if (!root["channel"].isString()) {
                std::stringstream ss;
                ss << "parse request error, "
                   << reader.getFormattedErrorMessages();
                throw std::runtime_error(ss.str());
                return;
            }

            channel = root["channel"].asString();
        }

        if ((event == EV_SUBSCRIBE) && (channel == CH_TRANSACTION)) {
            if (!root["address"].isString() && !root["address"].isArray()) {
                std::stringstream ss;
                ss << "parse request error, invalid address!"
                   << reader.getFormattedErrorMessages();
                throw std::runtime_error(ss.str());
                return;
            }

            std::vector<std::string> addresses;
            if (root["address"].isString()) {
                auto short_addr = root["address"].asString();
                if (!short_addr.empty()) {
                    auto pay_addr_str = get_address(short_addr);
                    auto pay_addr = wallet::payment_address(pay_addr_str);
                    if (!pay_addr) {
                        send_bad_response(nc, ("invalid did or address " + short_addr).c_str());
                        return;
                    }

                    addresses.push_back(pay_addr_str);
                }
            }
            else if (root["address"].isArray()) {
                auto array = root["address"];
                int length = (int)(array.size());
                for (int i = 0; i < length; ++i) {
                    auto item = array[i];
                    if (!item.isString()) {
                        send_bad_response(nc, "invalid did or address (not string).");
                        return;
                    }

                    auto short_addr = item.asString();
                    if (!short_addr.empty()) {
                        auto pay_addr_str = get_address(short_addr);
                        auto pay_addr = wallet::payment_address(pay_addr_str);
                        if (!pay_addr) {
                            send_bad_response(nc, ("invalid did or address " + short_addr).c_str());
                            return;
                        }

                        if (addresses.end() == std::find(addresses.begin(), addresses.end(), pay_addr_str)) {
                            addresses.push_back(pay_addr_str);
                        }
                    }
                }
            }
            else {
                send_bad_response(nc, "invalid did or address (not string).");
                return;
            }

            auto it = map_connections_.find(&nc);
            if (it != map_connections_.end()) {
                std::lock_guard<std::mutex> guard(subscribers_lock_);
                std::weak_ptr<struct mg_connection> week_con(it->second);
                auto sub_it = subscribers_.find(week_con);
                if (sub_it != subscribers_.end()) {
                    auto& sub_list = sub_it->second;

                    if (addresses.empty()) {
                        sub_list.clear();
                        send_response(nc, EV_SUBSCRIBED, channel);
                        return;
                    }

                    for (const auto& address : addresses) {
                        if (sub_list.end() == std::find(sub_list.begin(), sub_list.end(), address)) {
                            sub_list.push_back(address);
                        }
                    }

                    send_response(nc, EV_SUBSCRIBED, channel);
                }
                else {
                    string_vector sub_list;
                    for (const auto& address : addresses) {
                        if (sub_list.end() == std::find(sub_list.begin(), sub_list.end(), address)) {
                            sub_list.push_back(address);
                        }
                    }

                    subscribers_.insert({ week_con, sub_list });
                    send_response(nc, EV_SUBSCRIBED, channel);
                }
            }
            else {
                send_bad_response(nc, "connection lost.");
            }
        }
        else if ((event == EV_UNSUBSCRIBE) && (channel == CH_TRANSACTION)) {
            auto it = map_connections_.find(&nc);
            if (it != map_connections_.end()) {
                std::lock_guard<std::mutex> guard(subscribers_lock_);
                std::weak_ptr<struct mg_connection> week_con(it->second);
                subscribers_.erase(week_con);
                send_response(nc, EV_UNSUBSCRIBED, channel);
            }
            else {
                send_bad_response(nc, "no subscription.");
            }
        }

        else if (channel == CH_BLOCK || channel == CH_HEIGHT) {
            if (event == EV_SUBSCRIBE) {
                auto it = map_connections_.find(&nc);
                if (it != map_connections_.end()) {
                    std::lock_guard<std::mutex> guard(block_subscribers_lock_);
                    std::weak_ptr<struct mg_connection> week_con(it->second);

                    auto iter = block_subscribers_.find(week_con);
                    if (iter != block_subscribers_.end()) {
                        auto& sub_list = iter->second;
                        if (sub_list.end() == std::find(sub_list.begin(), sub_list.end(), channel)) {
                            sub_list.push_back(channel);
                        }

                        send_response(nc, EV_SUBSCRIBED, channel);
                    }
                    else {
                        string_vector sub_list;
                        sub_list.push_back(channel);

                        block_subscribers_.insert({ week_con, sub_list });
                        send_response(nc, EV_SUBSCRIBED, channel);
                    }
                }
                else {
                    send_bad_response(nc, "connection lost.");
                }
            }
            else if (event == EV_UNSUBSCRIBE) {
                auto it = map_connections_.find(&nc);
                if (it != map_connections_.end()) {
                    std::lock_guard<std::mutex> guard(block_subscribers_lock_);
                    std::weak_ptr<struct mg_connection> week_con(it->second);

                    auto iter = block_subscribers_.find(week_con);
                    if (iter != block_subscribers_.end()) {
                        auto& params = iter->second;
                        auto chit = std::find(params.begin(), params.end(), channel);
                        if (chit != params.end()) {
                            params.erase(chit);
                        }

                        if (params.empty()) {
                            subscribers_.erase(week_con);
                        }
                    }

                    send_response(nc, EV_UNSUBSCRIBED, channel);
                }
                else {
                    send_bad_response(nc, "no subscription.");
                }
            }
            else {
                send_bad_response(nc, "request not support.");
            }
        }

        else if (event == EV_VERSION) {
            Json::Value version;
            version["wallet"] = MVS_VERSION;
            version["protocol"] = node_.network_settings().protocol;
            send_response(nc, event, "", version);
        }

        else if (event == EV_PING) {
            Json::Value pong = "pong";
            send_response(nc, event, "", pong);
        }
        else {
            send_bad_response(nc, "request not support.");
        }
    }
    catch (std::exception& e) {
        auto error = std::string(e.what());
        log::info(NAME) << "on on_ws_frame_handler: " << error;
        send_bad_response(nc, error.c_str());
    }
}

void WsPushServ::on_close_handler(struct mg_connection& nc)
{
    if (is_websocket(nc))
    {
        map_connections_.erase(&nc);
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

std::string WsPushServ::get_address(const std::string& did_or_address) const
{
    auto& blockchain = node_.chain_impl();
    if (did_or_address.empty() || blockchain.is_valid_address(did_or_address)) {
        return did_or_address;
    }

    auto is_test = blockchain.chain_settings().use_testnet_rules;
    if (bc::blockchain::block_chain_impl::is_valid_did_symbol(did_or_address, !is_test)) {
        auto diddetail = blockchain.get_registered_did(did_or_address);
        if (diddetail) {
            return diddetail->get_address();
        }
    }

    return "";
}

}
