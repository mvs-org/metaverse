/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/network/proxy.hpp>

#define BOOST_BIND_NO_PLACEHOLDERS

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/const_buffer.hpp>
#include <metaverse/network/define.hpp>
#include <metaverse/network/socket.hpp>
#include <metaverse/bitcoin/utility/time.hpp>
#include <chrono>
#include <atomic>
#include <iostream>

namespace libbitcoin {
namespace network {

#ifndef NDEBUG
class traffic{
public:
	static traffic& instance();

	void rx(uint64_t bytes){
		rx_.fetch_add(bytes);
	}

	void tx(uint64_t bytes){
		tx_.fetch_add(bytes);
	}

	~traffic(){
		auto now = std::chrono::system_clock::now();
		std::cout << "it costs " << std::chrono::duration_cast<std::chrono::seconds>(now - start_time_).count() << " seconds, rx," << rx_.load() << ",tx," << tx_.load()  << std::endl;
	}
private:
	static boost::detail::spinlock spinlock_;
	static std::shared_ptr<traffic> instance_;
	traffic():rx_{0}, tx_{0}, start_time_{std::chrono::system_clock::now()}
	{
	}

private:
	std::atomic<uint64_t> rx_;
	std::atomic<uint64_t> tx_;
	std::chrono::system_clock::time_point start_time_;
};
traffic& traffic::instance(){
	if (instance_ == nullptr) {
		boost::detail::spinlock::scoped_lock guard{spinlock_};
		if (instance_ == nullptr) {
			instance_.reset(new traffic{});
		}
	}
	return *instance_;
}
boost::detail::spinlock traffic::spinlock_;
std::shared_ptr<traffic> traffic::instance_ = nullptr;
#endif

#define NAME "proxy"

using namespace message;
using namespace std::placeholders;

proxy::proxy(threadpool& pool, socket::ptr socket, uint32_t protocol_magic,
    uint32_t protocol_version)
  : protocol_magic_(protocol_magic),
    protocol_version_(protocol_version),
    authority_(socket->get_authority()),
    heading_buffer_(heading::maximum_size()),
    payload_buffer_(heading::maximum_payload_size(protocol_version_)),
	dispatch_{pool, "proxy"},
    socket_(socket),
    stopped_(true),
    peer_protocol_version_(message::version::level::maximum),
    message_subscriber_(pool),
    stop_subscriber_(std::make_shared<stop_subscriber>(pool, NAME)),
	processing_{false},
	misbehaving_{0}
{
}

proxy::~proxy()
{
    BITCOIN_ASSERT_MSG(stopped(), "The channel was not stopped.");
}

// Properties.
// ----------------------------------------------------------------------------

const config::authority& proxy::authority() const
{
    return authority_;
}

message::version proxy::version() const
{
    const auto version = peer_version_message_.load();
    BITCOIN_ASSERT_MSG(version, "Peer version read before set.");
    return *version;
}

void proxy::set_version(message::version::ptr value)
{
    peer_version_message_.store(value);
    peer_protocol_version_.store(value->value);
}

// Start sequence.
// ----------------------------------------------------------------------------

void proxy::start(result_handler handler)
{
    if (!stopped())
    {
        handler(error::operation_failed);
        return;
    }

    stopped_ = false;
    stop_subscriber_->start();
    message_subscriber_.start();

    // Allow for subscription before first read, so no messages are missed.
    handler(error::success);

    // Start the read cycle.
    read_heading();
}

// Stop subscription.
// ----------------------------------------------------------------------------

void proxy::subscribe_stop(result_handler handler)
{
    stop_subscriber_->subscribe(handler, error::channel_stopped);
}

// Read cycle (read continues until stop).
// ----------------------------------------------------------------------------

void proxy::read_heading()
{
    if (stopped())
        return;

    // The heading buffer is protected by ordering, not the critial section.

    // Critical Section (external)
    ///////////////////////////////////////////////////////////////////////////
    const auto socket = socket_->get_socket();

    using namespace boost::asio;
    async_read(socket->get(), buffer(heading_buffer_, heading_buffer_.size()),
        std::bind(&proxy::handle_read_heading,
            shared_from_this(), _1, _2));
    ///////////////////////////////////////////////////////////////////////////
}

void proxy::handle_read_heading(const boost_code& ec, size_t)
{
    if (stopped())
        return;

    // TODO: verify client quick disconnect.
    if (ec)
    {
        log::debug(LOG_NETWORK)
            << "Heading read failure [" << authority() << "] "
            << code(error::boost_to_error_code(ec)).message();
        stop(ec);
        return;
    }
#ifndef NDEBUG
    traffic::instance().rx(heading_buffer_.size());
#endif
    const auto head = heading::factory_from_data(heading_buffer_);

    if (!head.is_valid())
    {
        log::warning(LOG_NETWORK) 
            << "Invalid heading from [" << authority() << "]";
        stop(error::bad_stream);
        return;
    }

    if (head.magic != protocol_magic_)
    {
        log::warning(LOG_NETWORK)
            << "Invalid heading magic (" << head.magic << ") from ["
            << authority() << "]";
        stop(error::bad_stream);
        return;
    }

    if (head.payload_size > payload_buffer_.capacity())
    {
        log::warning(LOG_NETWORK)
            << "Oversized payload indicated by " << head.command
            << " heading from [" << authority() << "] ("
            << head.payload_size << " bytes)";
        stop(error::bad_stream);
        return;
    }

    read_payload(head);
    handle_activity();
}

void proxy::read_payload(const heading& head)
{
    if (stopped())
        return;

    // This does not cause a reallocation.
    payload_buffer_.resize(head.payload_size);

    // The payload buffer is protected by ordering, not the critial section.

    // Critical Section (external)
    ///////////////////////////////////////////////////////////////////////////
    const auto socket = socket_->get_socket();

    using namespace boost::asio;
    async_read(socket->get(), buffer(payload_buffer_, head.payload_size),
        std::bind(&proxy::handle_read_payload,
            shared_from_this(), _1, _2, head));
    ///////////////////////////////////////////////////////////////////////////
}

void proxy::handle_read_payload(const boost_code& ec, size_t payload_size,
    const heading& head)
{
    if (stopped())
        return;

    // TODO: verify client quick disconnect.
    if (ec)
    {
        log::debug(LOG_NETWORK)
            << "Payload read failure [" << authority() << "] "
            << code(error::boost_to_error_code(ec)).message();
        stop(ec);
        return;
    }

#ifndef NDEBUG
    traffic::instance().rx(payload_buffer_.size());
#endif

    auto checksum = bitcoin_checksum(payload_buffer_);
    if (head.checksum != checksum)
    {
        log::warning(LOG_NETWORK) 
            << "Invalid " << head.command << " payload from [" << authority()
            << "] bad checksum. size is " << payload_size;
        stop(error::bad_stream);
        return;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // TODO: we aren't getting a stream benefit if we read the full payload
    // before parsing the message. Should just make this a message parse.
    ///////////////////////////////////////////////////////////////////////////
    auto request = std::bind(&proxy::handle_request,
            this->shared_from_this(), payload_buffer_, peer_protocol_version_.load(), head, payload_size);
    {
    	scoped_lock lock{mutex_};
    	pendingRequests_.push(std::move(request));
    }
    dispatch();

    handle_activity();
    read_heading();
}

void proxy::dispatch()
{
	scoped_lock lock{mutex_};
	if(! processing_.load())
	{
		if(! pendingRequests_.empty())
		{
			auto req = std::move(pendingRequests_.front() );
			processing_.store(true);
			dispatch_.unordered(req);
			pendingRequests_.pop();
			return;
		}
	}
}

void proxy::handle_request(data_chunk payload_buffer, uint32_t peer_protocol_version, heading head, size_t payload_size)
{
	bool succeed = false;
	struct clean_up{
		~clean_up(){
			processing_.store(false);
			if(succeed_)
			{
				proxy_->dispatch();
				return;
			}
			proxy_->clear_request();
		}
		std::atomic_bool& processing_;
		bool& succeed_;
		proxy * proxy_;
	} clean_up_{processing_, succeed, this};

	// Notify subscribers of the new message.
	payload_source source(payload_buffer);
	payload_stream istream(source);
	const auto version = peer_protocol_version;
	const auto code = message_subscriber_.load(head.type(), version, istream);
	const auto consumed = istream.peek() == std::istream::traits_type::eof();

	if (code)
	{
		log::warning(LOG_NETWORK)
			<< "Invalid " << head.command << " payload from [" << authority()
			<< "] " << code.message();
		stop(code);
		return;
	}

	if (!consumed)
	{
		log::warning(LOG_NETWORK)
			<< "Invalid " << head.command << " payload from [" << authority()
			<< "] trailing bytes.";
		stop(error::bad_stream);
		return;
	}

	log::trace(LOG_NETWORK)
		<< "Valid " << head.command << " payload from [" << authority()
		<< "] (" << payload_size << " bytes)";
	succeed = true;
}

// Message send sequence.
// ----------------------------------------------------------------------------

void proxy::do_send(const std::string& command, const_buffer buffer,
    result_handler handler)
{
    if (stopped())
    {
        handler(error::channel_stopped);
        return;
    }

    //thin log network
    if (command != "getheaders" && command != "headers"){
        log::debug(LOG_NETWORK)
            << "Sending " << command << " to [" << authority() << "] ("
            << buffer.size() << " bytes)";
    }

    // Critical Section (protect socket)
    ///////////////////////////////////////////////////////////////////////////
    // The socket is locked until async_write returns.
    const auto socket = socket_->get_socket();

    // The shared buffer is kept in scope until the handler is invoked.
    using namespace boost::asio;
    async_write(socket->get(), buffer,
        std::bind(&proxy::handle_send,
            shared_from_this(), _1, buffer, handler));
    ///////////////////////////////////////////////////////////////////////////
}

void proxy::handle_send(const boost_code& ec, const_buffer buffer,
    result_handler handler)
{
    const auto error = code(error::boost_to_error_code(ec));

    if (error)
        log::trace(LOG_NETWORK)
            << "Failure sending " << buffer.size() << " byte message to ["
            << authority() << "] " << error.message();
    else{
#ifndef NDEBUG
    	traffic::instance().tx(buffer.size());
#endif
    }

    handler(error);
}

// Stop sequence.
// ----------------------------------------------------------------------------

// This is not short-circuited by a stop test because we need to ensure it
// completes at least once before invoking the handler. That would require a
// lock be taken around the entire section, which poses a deadlock risk.
// Instead this is thread safe and idempotent, allowing it to be unguarded.
void proxy::stop(const code& ec)
{
    BITCOIN_ASSERT_MSG(ec, "The stop code must be an error code.");

    stopped_ = true;

    // Prevent subscription after stop.
    message_subscriber_.stop();
    message_subscriber_.broadcast(error::channel_stopped);

    // Prevent subscription after stop.
    stop_subscriber_->stop();
    stop_subscriber_->relay(ec);

    // Give channel opportunity to terminate timers.
    handle_stopping();

    // The socket_ is internally guarded against concurrent use.
    socket_->close();
}

void proxy::stop(const boost_code& ec)
{
    stop(error::boost_to_error_code(ec));
}

bool proxy::stopped() const
{
    return stopped_;
}

std::map<config::authority, int64_t> proxy::banned_;

bool proxy::blacklisted(const config::authority& authority)
{
	auto it = banned_.find(authority);
	if(it != banned_.end())
	{
		auto millissecond = unix_millisecond();
		if (it->second >= millissecond)
		{
			return true;
		}
	}
	return false;
}


bool proxy::misbehaving(int32_t howmuch)
{
	misbehaving_ += howmuch;
	if (misbehaving_.load() >= 100)
	{
		{
			boost::detail::spinlock::scoped_lock guard{spinlock_};
			auto millissecond = unix_millisecond();
			banned_.insert({authority(), millissecond + 24 * 3600 * 1000});
		}
		log::debug(LOG_NETWORK) << "channel misbehave trigger," << authority();
		stop(error::bad_stream);
		return true;
	}
	return false;
}

} // namespace network
} // namespace libbitcoin
