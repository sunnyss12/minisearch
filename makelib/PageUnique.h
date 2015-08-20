#ifndef PAGE_UNIQUE_H
#define PAGE_UNIQUE_H

#include <boost/noncopyable.hpp>
#include <string>
#include <map>
#include <vector>
#include "Document.h"

class PageUnique : boost::noncopyable
{
public:
    PageUnique(std::string indexPath);

    void readIndex(); //读取索引
    void readDocuments();  //根据索引，依次读取文章

    void computeFrequency(); //每篇文章计算词频
    void computeTopK();  //每篇文章计算topK

    void unique();
    void saveToPageLib(); //保存磁盘

private:
    std::string indexPath_; //索引文件的路径
    std::map<int, std::pair<long, size_t> > index_; //索引的内容
    std::vector<Document> documents_; //文章集合
    std::vector<bool> bit_; //标志位，用于unique
};


#endif //PAGE_UNIQUE_H