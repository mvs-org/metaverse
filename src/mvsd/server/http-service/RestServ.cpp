#include "mvs/http/RestServ.hpp"

#include <mvs/http/Exception_instance.hpp>
#include <mvs/http/Stream_buf.hpp>
#include <exception>

using namespace http;

void RestServ::reset(HttpMessage& data) noexcept
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

    log::debug(LOG_HTTP)<<"ws snd len "<<len<<" msg:["<<msg<<"]";
    for (iter = mg_next(nc.mgr, nullptr); iter != nullptr; iter = mg_next(nc.mgr, iter))
    {
      mg_send_websocket_frame(iter, WEBSOCKET_OP_TEXT, msg, len);
    }
}
void RestServ::websocketSend(mg_connection* nc, const char* msg, size_t len) 
{
    log::debug(LOG_HTTP)<<"ws snd len "<<len<<" msg:["<<msg<<"]";
    mg_send_websocket_frame(nc, WEBSOCKET_OP_TEXT, msg, len);
}


// --------------------- websocket interface -----------------------
void RestServ::websocketBroadcast(mg_connection& nc, WebsocketMessage ws) 
{
    using namespace bc;

    std::ostringstream sout;
    std::istringstream sin;
    //process here
    try{
        ws.data_to_arg();
        sin.str(ws.mvs_params());
        const char* mm = ws.mvs_method().c_str();

        explorer::dispatch_command(1, &mm, 
            sin, sout, sout);

    }catch(...){
        log::error(LOG_HTTP)<<__func__<<":"<<sout.rdbuf();
    }

    //websocketBroadcast(nc, sout.str().c_str(), sout.str().size() - 1);
    websocketSend(&nc, sout.str().c_str(), sout.str().size());
}

// --------------------- json rpc interface -----------------------
void RestServ::httpRpcRequest(mg_connection& nc, HttpMessage data)
{
    using namespace mg;
    using namespace bc::client;
    using namespace bc::protocol;

    reset(data);
    log::debug(LOG_HTTP)<<"req uri:["<<uri_.top()<<"] body:["<<data.body()<<"]";

    StreamBuf buf{nc.send_mbuf};
    out_.rdbuf(&buf);
    out_.reset(200, "OK");
    try {
    	if (uri_.empty() || uri_.top() != "rpc") {
    	    throw ForbiddenException{"URI not support"};
    	}

        //process here
        data.data_to_arg();

        std::stringstream sout;
        std::istringstream sin;
        sin.str(data.mvs_params());
        const char* mm = data.mvs_method().c_str();

        bc::explorer::dispatch_command(1, &mm,
            sin, sout, sout);

        log::debug(LOG_HTTP)<<"cmd result:"<<sout.rdbuf();

    	out_<<sout.str();

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

// --------------------- Restful-api interface -----------------------
void RestServ::httpRequest(mg_connection& nc, HttpMessage data)
{
    using namespace mg;
    using namespace bc::client;
    using namespace bc::protocol;

    reset(data);

    log::debug(LOG_HTTP)<<"req uri:["<<uri_.top()<<"] body:["<<data.body()<<"]";

    StreamBuf buf{nc.send_mbuf};
    out_.rdbuf(&buf);
    out_.reset(200, "OK");

    try {
        if (uri_.empty() || uri_.top() != "api") {
        	throw ForbiddenException{"URI not support"};
        }
        uri_.pop();

        if (!uri_.empty()) {
            data.data_to_arg();
            // let uri as method
            data.set_mvs_method({uri_.top().data(), uri_.top().size()});

            //process here
            std::stringstream sout;
            std::istringstream sin;
            sin.str(data.mvs_params());
            const char* mm = data.mvs_method().c_str();

            bc::explorer::dispatch_command(1, &mm,
                sin, sout, sout);

            log::debug(LOG_HTTP)<<"sout:"<<sout.rdbuf();

        	out_<<sout.str();
            state_|= MatchUri;
            state_|= MatchMethod;

        }

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

