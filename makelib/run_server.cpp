#include "QueryServer.h"
using namespace muduo;
using namespace muduo::net;

int main(int argc, char const *argv[])
{
    EventLoop loop;
    InetAddress addr("192.168.1.117", 9981);
    QueryServer server(&loop, addr);
    server.start();

    loop.loop();

    return 0;
}
