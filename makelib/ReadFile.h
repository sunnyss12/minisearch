#ifndef READ_FILE_H
#define READ_FILE_H

#include <boost/noncopyable.hpp>
#include <string>
#include "RobustIO.h"

class ReadFile : boost::noncopyable
{
public:
    ReadFile(const std::string &path);
    ~ReadFile();

    size_t readnBytes(char *usrbuf, size_t count);
    size_t readLine(char *usrbuf, size_t maxlen);
    std::string readnBytesAsString(size_t count);
    std::string readLineAsString();

    bool end();   //判断当前位置是否到达文件尾
    long seek() const;  //获取当前偏移量
    void seekBeg(long offset); //从文件开头偏移的量
    void seekCur(long offset); //从当前位置偏移
    void seekEnd(long offset); //从文件末尾偏移

    //重载bool转化，用于判断文件打开是否成功
    operator bool() const
    { return fd_ != -1; }

private:
    const int fd_;
    off_t filesize_;
    RobustIO robustIO_;
};


#endif //READ_FILE_H
