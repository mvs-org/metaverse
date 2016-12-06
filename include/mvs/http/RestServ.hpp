#include "Mongoose.hpp"

#include <mvs/http/Tokeniser.hpp>
#include <mvs/http/Exception_instance.hpp>
#include <mvs/http/Stream_buf.hpp>

#include <bitcoin/client.hpp>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/consensus/miner.hpp> //miner

namespace http{

using namespace mg;
using namespace bc;

class RestServ : public mg::Mgr<RestServ>
{
public:
    explicit RestServ(const char* webroot, blockchain::block_chain_impl& rhs, consensus::miner& miner)
        :socket_(context_, protocol::zmq::socket::role::dealer), blockchain_(rhs), miner_(miner)
    {
        memset(&httpoptions_, 0x00, sizeof(httpoptions_));
        httpoptions_.document_root = webroot;

        socket_.connect({"tcp://127.0.0.1:9091"});
    }
    ~RestServ() noexcept {};

    // Copy.
    RestServ(const RestServ& rhs) = delete;
    RestServ& operator=(const RestServ& rhs) = delete;

    // Move.
    RestServ(RestServ&&) = delete;
    RestServ& operator=(RestServ&&) = delete;

    //reset
    void reset(mg::HttpMessage& data) noexcept;

    //request
    void httpStatic (mg_connection& nc, HttpMessage data);
    void httpRequest (mg_connection& nc, HttpMessage data);
    void httpRpcRequest (mg_connection& nc, HttpMessage data);
    void websocketBroadcast (mg_connection& nc, const char* msg, size_t len);
    void websocketSend(mg_connection* nc, const char* msg, size_t len);
    void websocketSend(mg_connection& nc, WebsocketMessage ws);

    // http session
    bool user_auth(mg_connection& nc, HttpMessage data);
    mg_serve_http_opts& get_httpoptions(){return httpoptions_;}
    std::shared_ptr<Session> get_from_session_list(HttpMessage data);
    std::shared_ptr<Session> push_session(HttpMessage data);
    bool check_sessions();

    std::pair<std::string_view,std::string_view> user_check(mg::HttpMessage &data)
    {
       auto user = data.header("user");
       auto password = data.header("pass");
    
       if (user.empty() || password.empty()) {
      	    throw std::logic_error("authorisation required");
       }
       return std::make_pair(user, password);
    }

    
private:
    enum : int {
      // Method values are represented as powers of two for simplicity.
      MethodGet = 1 << 0,
      MethodPost = 1 << 1,
      MethodPut = 1 << 2,
      MethodDelete = 1 << 3,
      // Method value mask.
      MethodMask = MethodGet | MethodPost | MethodPut | MethodDelete,
    
      // Subsequent bits represent matching components.
      MatchMethod = 1 << 4,
      MatchUri = 1 << 5,
      // Match result mask.
      MatchMask = MatchMethod | MatchUri
    };

    bool isSet(int bs) const noexcept { return (state_ & bs) == bs; }

    // zmq
    protocol::zmq::context context_;
    protocol::zmq::socket socket_;

    // http
    mg_serve_http_opts httpoptions_;
    constexpr static const double SESSION_TTL = 90.0;
    std::list< std::shared_ptr<Session> > session_list_;

    //miner

    // config
    mg::OStream out_;
    Tokeniser<'/'> uri_;
    int state_{0};
    const char* const servername_{"Http-Metaverse"};
    blockchain::block_chain_impl& blockchain_;
    consensus::miner& miner_;
};

} // http
