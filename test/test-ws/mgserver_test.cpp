
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <metaverse/mgbubble/MgServer.hpp>

using namespace mgbubble;

class Event {
public:
    void operator()(uint64_t id) 
    {
        if (callback_)
            callback_(id);
        
        auto tmp = std::move(callback_);
    }

    // called on mongoose thread
    void callback(const std::function<void(uint64_t)>&& handler)
    {
        callback_ = std::move(handler);
    }

private:

    std::function<void(uint64_t id)> callback_;
};

class EventServer : public MgServer{
public:
    explicit EventServer(const std::string& svr_addr) : MgServer(svr_addr){}
    bool start() {
        if (!attach_notify())
            return false;
        return base::start();
    }

    void spawn_to_mongoose(const std::function<void(uint64_t)> handler) {
        static uint64_t id = 0;
        auto pmsg = std::make_shared<Event>();
        auto evid = ++id;
        struct mg_event ev { evid, pmsg.get() };
        if (notify(ev))
        {
            pmsg->callback([pmsg, handler](uint64_t id) { handler(id); });
        }
    }

    void on_notify_handler(struct mg_connection& nc, struct mg_event& ev) {
        if (ev.data == nullptr)
            return;
        auto& msg = *(WsEvent*)ev.data;
        msg(ev.id);
    }
};

BOOST_AUTO_TEST_SUITE(mgserver_tests)

BOOST_AUTO_TEST_CASE(event_notify)
{
    EventServer server("127.0.0.1:8821");
    bool ret = server.start();
    BOOST_REQUIRE_EQUAL(ret, true);

    for (auto i = 0; i < 5; ++i)
    {
        std::thread([this, i]() {
            constexpr int test = 10000;
            for(auto k=0; k<test; ++k)
            {
                server.spawn_to_mongoose([i, k](size_t id) {
                    log::info("TEST") << "[" << i << " - " << k << "] on spawn_to_mongoose: " << id;
                });
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }).detach();
    }
}

BOOST_AUTO_TEST_SUITE_END()
#endif

