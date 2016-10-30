#include "mvs-http/Mongoose.hpp"
#include "mvs-http/RestServ.hpp"



int main(int argc, char** argv)
{

    http::RestServ rs{"./web"};
    auto& conn = rs.bind("8821");
    mg_set_protocol_http_websocket(&conn);
    for(;;)
        rs.poll(1000);


    return 0;
}
