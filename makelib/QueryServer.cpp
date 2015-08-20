#include "QueryServer.h"
#include "SegmentSingleton.h"
#include <muduo/base/Logging.h>
using namespace std;
using namespace std::placeholders;

QueryServer::QueryServer(muduo::net::EventLoop *loop,
                         const muduo::net::InetAddress &addr)
: server_(loop, addr, "QueryServer")
{
    server_.setMessageCallback(std::bind(&QueryServer::onMessage, this, _1, _2, _3));
    query_.readPageLibIndex();
    query_.readInvertedIndex();

    SegmentSingleton::getInstance();
}

void QueryServer::start()
{
    server_.start();
    LOG_INFO << "查询服务器已经启动";
}

void QueryServer::onMessage(const muduo::net::TcpConnectionPtr &conn, 
                   muduo::net::Buffer *buf, 
                   muduo::Timestamp)
{
    muduo::string s(buf->retrieveAllAsString());
    LOG_INFO << "receive php msg: " << s;
    std::string msg(s.c_str());

    std::string result = query_.queryPage(msg);

    conn->send(result.c_str());
}