
#include <metaverse/mgbubble/Mongoose.hpp>
#include <metaverse/mgbubble/MgServer.hpp>
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

class HttpServ : public MgServer
{
    typedef MgServer base;
public:
    explicit HttpServ(const char* webroot, libbitcoin::server::server_node &node, const std::string& srv_addr)
        : node_(node), MgServer(srv_addr)
    {
        document_root_ = webroot;
        set_document_root(document_root_.c_str());
    }
    ~HttpServ() noexcept { stop(); };

    // Copy.
    HttpServ(const HttpServ& rhs) = delete;
    HttpServ& operator=(const HttpServ& rhs) = delete;

    // Move.
    HttpServ(HttpServ&&) = delete;
    HttpServ& operator=(HttpServ&&) = delete;

    void rpc_request(mg_connection& nc, HttpMessage data, uint8_t rpc_version = 1);
    void ws_request(mg_connection& nc, WebsocketMessage ws);

public:
    void reset(HttpMessage& data) noexcept;

    bool start() override;

    void spawn_to_mongoose(const std::function<void(uint64_t)>&& handler);

protected:
    void run() override;

    void on_http_req_handler(struct mg_connection& nc, struct http_message& msg) override;
    void on_notify_handler(struct mg_connection& nc, struct mg_event& ev) override;
    void on_ws_handshake_done_handler(struct mg_connection& nc) override;
    void on_ws_frame_handler(struct mg_connection& nc, struct websocket_message& msg) override;

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

    // config
    static thread_local OStream out_;
    static thread_local Tokeniser<'/'> uri_;
    static thread_local int state_;
    const char* const servername_{"Metaverse " MVS_VERSION};
    libbitcoin::server::server_node &node_;
    string document_root_;
};

} // mgbubble
