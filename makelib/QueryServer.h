#ifndef QUERY_SERVER_H
#define QUERY_SERVER_H

#include <boost/noncopyable.hpp>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Timestamp.h>
#include "PageQuery.h"

class QueryServer : boost::noncopyable
{
public:
    QueryServer(muduo::net::EventLoop *loop,
                const muduo::net::InetAddress &addr);

    void start();
private:
    void onMessage(const muduo::net::TcpConnectionPtr &conn, 
                   muduo::net::Buffer *buf, 
                   muduo::Timestamp);

    muduo::net::TcpServer server_;
    PageQuery query_;
};

#endif //QUERY_SERVER_H