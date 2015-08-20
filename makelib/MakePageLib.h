#ifndef MAKE_PAGE_LIB_H
#define MAKE_PAGE_LIB_H

#include <boost/noncopyable.hpp>
#include <string>
#include "TruncFile.h"

class MakePageLib : boost::noncopyable
{
public:
    MakePageLib(std::string dataDirectory,
                const std::string &pagelib,
                const std::string &indexfile);

    void traversDirectoryWithRecursion(const std::string &path);
    void traversDirectory();
    void processFile(const std::string &filename);

private:
    int docid_;  //用于计数
    std::string dataDirectory_; //数据目录
    TruncFile pagelib_;
    TruncFile index_;
};


#endif //MAKE_PAGE_LIB_H