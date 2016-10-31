#include "mvs/http/RestServ.hpp"

#include <mvs/http/Exception_instance.hpp>
#include <mvs/http/Stream_buf.hpp>
#include <exception>

using namespace http;

void RestServ::reset(HttpMessage data) noexcept
{
  state_ = 0;

  const auto method = data.method();
  if (method == "GET") {
    state_ |= MethodGet;
  } else if (method == "POST") {
    state_ |= MethodPost;
  } else if (method == "PUT") {
    state_ |= MethodPut;
  } else if (method == "DELETE") {
    state_ |= MethodDelete;
  }

  auto uri = data.uri();
  // Remove leading slash.
  if (uri.front() == '/') {
    uri.remove_prefix(1);
  }
  uri_.reset(uri);
}

void RestServ::httpStatic(mg_connection& nc, HttpMessage data)
{
    mg_serve_http(&nc, data.get(), httpoptions_);
}

void RestServ::websocketBroadcast(mg_connection& nc, const char* msg, size_t len) 
{
  mg_connection* iter;

  log::debug(LOG_HTTP)<<"websock msg:"<<msg;
  for (iter = mg_next(nc.mgr, nullptr); iter != nullptr; iter = mg_next(nc.mgr, iter))
  {
    mg_send_websocket_frame(iter, WEBSOCKET_OP_TEXT, msg, len);
  }
}

void RestServ::websocketBroadcast(mg_connection& nc, WebsocketMessage ws) 
{
    using namespace bc;
    //process here

    std::ostringstream ss;
    bc::explorer::dispatch_command(ws.argc(), const_cast<const char**>(ws.argv()), bc::cin, ss, ss);

//    mg_send_websocket_frame(&nc, WEBSOCKET_OP_TEXT, ss.str().c_str(), ss.str().size());
    websocketBroadcast(nc, ss.str().c_str(), ss.str().size() - 1);
}

void RestServ::httpRequest(mg_connection& nc, HttpMessage data)
{
    using namespace mg;
    using namespace bc::client;
    using namespace bc::protocol;

    reset(data);
    log::debug(LOG_HTTP)<<"req uri:["<<uri_.top()<<"] body:["<<data.body()<<"]";

    if (uri_.empty() || uri_.top() != "api") {
        StreamBuf buf{nc.send_mbuf};
        out_.rdbuf(&buf);
        out_.reset(200, "OK");
        out_<<"chen hao test-error rpc top";
        out_.setContentLength(); 
        return;
    }
    uri_.pop();

    StreamBuf buf{nc.send_mbuf};
    out_.rdbuf(&buf);
    out_.reset(200, "OK");

    const auto completion_handler = [this](size_t height)
    {
        this->out_<<"{\"result\":"<<height<<"}";
        this->out_.setContentLength(); 
    };

    const auto error_handler = [](const code& code)
    {
        std::cout << "error: " << code.message() << std::endl;
    };

    const auto unknown_handler = [](const std::string& command)
    {
        std::cout << "unknown command: " << command << std::endl;
    };

    auto func = [&](){
    	socket_stream stream(socket_);
        proxy proxy(stream, unknown_handler, 2000, 0);

    	// Make the request.
    	proxy.blockchain_fetch_last_height(error_handler, completion_handler);

    	zmq::poller poller;
    	poller.add(socket_);

    	// Wait 1 second for the response.
    	if (poller.wait(1000).contains(socket_.id()))
    	    stream.read(proxy); 
    };

    func();
    return;

    try {
    const auto body = data.body();
    if (!body.empty()) {
//      if (!request_.parse(data.body())) {
        throw BadRequestException{"request body is incomplete"_sv};
 //     }
    }
//    restRequest(data, now);
    if (!isSet(MatchUri)) {
      throw NotFoundException{errMsg() << "resource '" << data.uri() << "' does not exist"};
    }
    if (!isSet(MatchMethod)) {
      throw MethodNotAllowedException{errMsg() << "method '" << data.method()
          << "' is not allowed"};
    }
  } catch (const ServException& e) {
    out_.reset(e.httpStatus(), e.httpReason());
    out_ << e;
  } catch (const std::exception& e) {
    const int status{500};
    const char* const reason{"Internal Server Error"};
    out_.reset(status, reason);
    ServException::toJson(status, reason, e.what(), out_);
  }
  out_.setContentLength(); 
}
