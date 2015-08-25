#include "ReadFile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <muduo/base/Logging.h>

namespace
{
int openReadFile(const std::string &path)
{
    int fd = ::open(path.c_str(), O_RDONLY);
    if(fd == -1)
    {
        LOG_ERROR << "open file failed. : " << path;
    }
    return fd;
}
off_t getFileSize(int fd)
{
    struct stat buf;
    if(fstat(fd,&buf) == -1)
    {
        LOG_ERROR <<"fstat failed";
    }
    LOG_INFO<<"filesize:"<<buf.st_size;
    return buf.st_size;

}
}

ReadFile::ReadFile(const std::string &path)
:   fd_(openReadFile(path)),
    filesize_(getFileSize(fd_)),
    robustIO_(fd_)
{
}

ReadFile::~ReadFile()
{
    if(fd_ != -1)
        ::close(fd_);
}

size_t ReadFile::readnBytes(char *usrbuf, size_t count)
{
    int nread = robustIO_.readn(usrbuf, count);
    if(nread == -1)
        LOG_SYSFATAL << "read error";

    return static_cast<size_t>(nread);
}

size_t ReadFile::readLine(char *usrbuf, size_t maxlen)
{
    int nread = robustIO_.readLine(usrbuf, maxlen);
    if(nread == -1)
        LOG_SYSFATAL << "readLine error";

    return static_cast<size_t>(nread);
}

//读取n个字节，作为字符串返回
std::string ReadFile::readnBytesAsString(size_t count)
{
    std::string result = "";

    char tmp[1024] = {0};
    const size_t kSize = sizeof(tmp);
    size_t nleft = count; //剩余的字节数
    while(nleft > 0)
    {
        size_t nbytes = (nleft < kSize) ? nleft : kSize;
        size_t ret = readnBytes(tmp, nbytes);
        result.append(tmp, ret);
        nleft -= ret;

        if(ret < nbytes) //EOF
            break;
    }

    return result;
}

//按行读取字符串
std::string ReadFile::readLineAsString()
{
    std::string result = "";

    char tmp[1024];
    const size_t kMaxLen = sizeof(tmp);
    while(1)
    {
        size_t ret = readLine(tmp, kMaxLen);
        //string& append (const char* s, size_t n);
        result.append(tmp, ret);
        if(ret < kMaxLen-1 || 
            (ret == kMaxLen-1 && tmp[ret-1] == '\n'))
            break;
    }

    return result;
}

bool ReadFile::end()
{
    return seek() == filesize_;
}
long ReadFile::seek() const
{
    long offset;
    if((offset = lseek(fd_, 0, SEEK_CUR))== -1)
        LOG_SYSFATAL << "lseek error";
    return offset;
} 

void ReadFile::seekBeg(long offset)
{
    if(lseek(fd_, offset, SEEK_SET) == -1)
        LOG_SYSFATAL << "lseek error";
    robustIO_.reset();
}

void ReadFile::seekCur(long offset)
{
    if(lseek(fd_, offset, SEEK_CUR) == -1)
        LOG_SYSFATAL << "lseek error";
    robustIO_.reset();
}

void ReadFile::seekEnd(long offset)
{
    if(lseek(fd_, offset, SEEK_END) == -1)
        LOG_SYSFATAL << "lseek error";
    robustIO_.reset();
}
