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

    void readIndex(); //读取索引,每行一条索引，索引的结构为：文章id,文章在pagelib的位置，文章的长度
    void computeWordFrequency();  //根据索引，依次读取文章,并对每篇文档分词，并所有分词的词频
    void computeWordWeightTopK(); //根据每篇文章的分词和词频，计算分词的权重，然后得到权重的TopK
    void unique();  //根据每篇文章分词权重的TopK计算simhash，并根据simhash来判断文章是否相似，如果相似将文章剔除
    void saveToPageLib(); //将没有相似的所有文章重新保存到磁盘

private:
    std::string indexPath_; //索引文件的路径
    std::map<int, std::pair<long, size_t> > index_; //索引的内容
    std::vector<Document> documents_; //文章集合
    std::vector<bool> bit_; //标志位，用于unique
};


#endif //PAGE_UNIQUE_H
