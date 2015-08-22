#include "TruncFile.h"
#include <muduo/base/Logging.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

namespace
{
int openTruncFile(const std::string &path)
{
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(fd == -1)
    {
        LOG_ERROR << "open trunc file failed. : " << path << " error :" << strerror(errno);
    }
    return fd;
}

ssize_t writen(int fd, const void *buf, size_t count)
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
}


TruncFile::TruncFile(const std::string &path)
:   fd_(openTruncFile(path))
{

}

TruncFile::~TruncFile()
{
    if(fd_ != -1)
        ::close(fd_);
}


size_t TruncFile::writenBytes(const char *usrbuf, size_t count)
{
    int nwrite = writen(fd_, usrbuf, count);
    if(nwrite == -1)
        LOG_SYSFATAL << "write error";

    return static_cast<size_t>(nwrite);
}

void TruncFile::writeString(const std::string &s)
{
    writenBytes(s.c_str(), s.size());
}

void TruncFile::writeStringLine(const std::string &s)
{
    writenBytes(s.c_str(), s.size());
    writenBytes("\r\n", strlen("\r\n"));
}


long TruncFile::seek() const
{
    long offset;
    if((offset = lseek(fd_, 0, SEEK_CUR))== -1)
        LOG_SYSFATAL << "lseek error";
    return offset;
} 

void TruncFile::seekBeg(long offset)
{
    if(lseek(fd_, offset, SEEK_SET) == -1)
        LOG_SYSFATAL << "lseek error";
}

void TruncFile::seekCur(long offset)
{
    if(lseek(fd_, offset, SEEK_CUR) == -1)
        LOG_SYSFATAL << "lseek error";
}

void TruncFile::seekEnd(long offset)
{
    if(lseek(fd_, offset, SEEK_END) == -1)
        LOG_SYSFATAL << "lseek error";
}
