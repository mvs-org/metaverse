#include "mvs/http/RestServ.hpp"

#include <mvs/http/Exception_instance.hpp>
#include <mvs/http/Stream_buf.hpp>
#include <exception>
#include <functional> //hash

namespace http{

using namespace mg;

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
void RestServ::websocketSend(mg_connection& nc, WebsocketMessage ws) 
{
    using namespace bc;

    //process here
    std::ostringstream sout;
    std::istringstream sin;
    try{
        ws.data_to_arg();

        explorer::dispatch_command(ws.argc(), const_cast<const char**>(ws.argv()), 
            sin, sout, sout, blockchain_);

    }catch(std::logic_error e){
        sout<<"{\"error\":\""<<e.what()<<"\"}";
    }catch(...){
        log::error(LOG_HTTP)<<__func__<<":"<<sout.rdbuf();
        sout<<"{\"error\":\"fatel error\"}";
    }

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
        bc::explorer::dispatch_command(data.argc(), const_cast<const char**>(data.argv()), 
            sin, sout, sout, blockchain_);

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
            // method
            data.add_arg({uri_.top().data(), uri_.top().size()});

            // username
            if (uri_.top() != "getnewaccount"_sv) {
                auto ret = get_from_session_list(data.get());
                if (!ret) throw std::logic_error{"nullptr for seesion"};
                data.add_arg(std::string(ret->user));
            }

            data.data_to_arg();
            // let uri as method

            //process here
            std::stringstream sout;
            std::istringstream sin;

            bc::explorer::dispatch_command(data.argc(), const_cast<const char**>(data.argv()), 
                sin, sout, sout, blockchain_);

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

std::shared_ptr<Session> RestServ::push_session(HttpMessage data)
{
    char user[64]{0x00}, pass[64]{0x00};
    auto ul = mg_get_http_var(&(data.get()->body), "user", user, sizeof(user));
    auto pl = mg_get_http_var(&(data.get()->body), "pass", pass, sizeof(pass));

    auto s = std::make_shared<Session>();

    s->created = s->last_used = mg_time();
    s->user = std::string(user, ul);
    s->pass = std::string(pass, pl);

    s->id = std::hash<std::shared_ptr<Session>>()(s);
    session_list_.push_back(s);

    log::debug("session")<<s->id<<" pushed";

    return s;
}

std::shared_ptr<Session> RestServ::get_from_session_list(HttpMessage data)
{
    mg_str* cookie_header = mg_get_http_header(data.get(), "cookie");;
    if (cookie_header == nullptr) 
        return nullptr;

    char ssid[21]{0x00};
    if (!mg_http_parse_header(cookie_header, SESSION_COOKIE_NAME, ssid, sizeof(ssid)))
        return nullptr;

    auto sid = std::stol(ssid, nullptr, 10);

    auto ret = std::find_if(session_list_.begin(), session_list_.end(), [&sid](std::shared_ptr<Session> p){
            return sid == p->id;
            });

    if (ret == session_list_.end())
        return nullptr;

    (*ret)->last_used = mg_time();
    return *ret;
}

bool RestServ::check_sessions()
{
    auto threshold = mg_time() - SESSION_TTL;

    for (auto iter = session_list_.begin(); iter != session_list_.end(); ++iter)
    {
        if ( (*iter)->last_used < threshold )
        {
            log::debug("session")<<(*iter)->id<<" removed";
            iter = session_list_.erase(iter);
        }
    }
    return true;
}

bool RestServ::user_auth(mg_connection& nc, HttpMessage data)
{
    char user[64]{0x00}, pass[64]{0x00};
    auto ul = mg_get_http_var(&(data.get()->body), "user", user, sizeof(user));
    auto pl = mg_get_http_var(&(data.get()->body), "pass", pass, sizeof(pass));

    try{
        if (ul > 0 && pl > 0){
            blockchain_.is_account_passwd_valid(std::string(user, ul), std::string(pass, pl));
        }else{
            throw std::logic_error{"Bad Request,user, pass required."};
        }

    }catch(std::exception& e){
        reset(data);
        StreamBuf buf{nc.send_mbuf};
        out_.rdbuf(&buf);
        out_.reset(403, "Forbidden");
        out_<<"{\"error\":\""<<e.what()<<"\"}";
        out_.setContentLength(); 

        return false;
    }

    return true;
}

}// namespace http

