#ifndef TRUNC_FILE_H
#define TRUNC_FILE_H

#include <boost/noncopyable.hpp>
#include <string>
#include "RobustIO.h"

class TruncFile : boost::noncopyable
{
public:
    TruncFile(const std::string &path);
    ~TruncFile();

    size_t writenBytes(const char *usrbuf, size_t count);
    void writeString(const std::string &s);
    void writeStringLine(const std::string &s);

    long seek() const;  //获取当前偏移量
    void seekBeg(long offset); //从文件开头偏移的量
    void seekCur(long offset); //从当前位置偏移
    void seekEnd(long offset); //从文件末尾偏移

    operator bool() const
    { return fd_ != -1; }

private:
    const int fd_;
};

#endif //TRUNC_FILE_H