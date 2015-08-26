#include "RobustIO.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)

//const int RobustIO::kBufferSize = 65536;

RobustIO::RobustIO(int fd)
{
    fd_ = fd;
    cnt_ = 0;
    ptr_ = buffer_;
    ::memset(buffer_, 0, kBufferSize);
}

ssize_t RobustIO::read(char *usrbuf, size_t n)
{
    //缓冲区为空时，执行read操作
    while(cnt_ <= 0)
    {
        ssize_t nread = ::read(fd_, buffer_, kBufferSize);
        if(nread == -1)
        {
            if(errno == EINTR)
                continue;
            return -1;  //ERROR
        }
        else if(nread == 0)
            return 0;

        //正常读取
        cnt_ = nread;
        ptr_ = buffer_; //重置指针
    }

    //现有库存和用户要求的数量 取较小者
    size_t cnt = (static_cast<size_t>(cnt_) < n) ? cnt_ : n;
    ::memcpy(usrbuf, ptr_, cnt);
    ptr_ += cnt;
    cnt_ -= cnt;

    return cnt;  //成功读取的字节数
}

ssize_t RobustIO::readn(char *usrbuf, size_t count)
{
    size_t nleft = count;  //剩余的字节数
    ssize_t nread; //用作返回值
    char *bufp = (char*)usrbuf; //缓冲区的偏移量

    while(nleft > 0)
    {
        //不再执行read系统调用
        nread = this->read(bufp, nleft);
        if(nread == -1)
        {
            if(errno == EINTR)
                continue;
            return -1; // ERROR
        }
        else if(nread == 0) //EOF
            break;

        nleft -= nread;
        bufp += nread;
    }

    return (count - nleft);
}

//碰不到换行符 返回maxlen-1
//遇到换行符 返回的是读取的字节数，包括\n
ssize_t RobustIO::readLine(char *usrbuf, size_t maxlen)
{
    size_t i; //计数
    int nread;

    char *bufp = usrbuf;
    char c; //暂存字符

    for(i = 0; i < maxlen - 1; ++i)
    {
        if((nread = this->read(&c, 1)) == -1)
            return -1;
        else if(nread == 0) //EOF
        {
            if(i == 0)
                return 0;
            break;
        }

        *bufp++ = c; //放入usrbuf
        if(c == '\n') //碰到换行符直接退出循环
            break; 
    }
    *bufp = '\0';
    if( i == maxlen-1)  //已经加了1
        return i;
    else
        return i+1;   //i表示遇到\n退出，这时i表示index，应该加1才表示读取的字节数.
}

ssize_t RobustIO::writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwrite;
    const char *bufp = (const char*)buf;
    
    while(nleft > 0)
    {
        nwrite = write(fd, bufp, nleft);
        if(nwrite <= 0) // ERROR
        {
            if(nwrite == -1 && errno == EINTR)
                continue;
            return -1;
        }

        nleft -= nwrite;
        bufp += nwrite;
    }
    
    return count;
}


void RobustIO::reset()
{
    cnt_ = 0;
    ptr_ = buffer_;
    ::memset(buffer_, 0, kBufferSize);
}
