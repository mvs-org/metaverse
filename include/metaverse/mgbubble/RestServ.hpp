
#include <metaverse/mgbubble/Mongoose.hpp>
#include <metaverse/mgbubble/utility/Stream_buf.hpp>
#include <metaverse/mgbubble/utility/Tokeniser.hpp>
#include <metaverse/mgbubble/exception/Instances.hpp>

#include <metaverse/client.hpp>
#include <metaverse/blockchain.hpp>
#include <metaverse/server/services/query_service.hpp> //public_query

namespace libbitcoin{
namespace server{
	class server_node;
}
}

namespace mgbubble{

using namespace bc;

class RestServ : public Mgr<RestServ>
{
public:
    explicit RestServ(const char* webroot, libbitcoin::server::server_node &node)
        :socket_(context_, protocol::zmq::socket::role::dealer), node_(node)
    {
        memset(&httpoptions_, 0x00, sizeof(httpoptions_));
        document_root_ = webroot;	
        httpoptions_.document_root = document_root_.c_str();

        socket_.connect(server::query_service::public_query);
    }
    ~RestServ() noexcept {};

    // Copy.
    RestServ(const RestServ& rhs) = delete;
    RestServ& operator=(const RestServ& rhs) = delete;

    // Move.
    RestServ(RestServ&&) = delete;
    RestServ& operator=(RestServ&&) = delete;

    //reset
    void reset(HttpMessage& data) noexcept;

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
    bool remove_from_session_list(HttpMessage data);
    std::shared_ptr<Session> push_session(HttpMessage data);
    bool check_sessions();

    std::pair<string_view,string_view> user_check(HttpMessage &data)
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
#if MVS_DEBUG
    constexpr static const double SESSION_TTL = 900.0;
#else
    constexpr static const double SESSION_TTL = 240.0;
#endif
    std::list< std::shared_ptr<Session> > session_list_;

    // config
    static thread_local OStream out_;
    static thread_local Tokeniser<'/'> uri_;
    static thread_local int state_;
    const char* const servername_{"Http-Metaverse"};
    libbitcoin::server::server_node &node_;
    string document_root_;
};

} // mgbubble
