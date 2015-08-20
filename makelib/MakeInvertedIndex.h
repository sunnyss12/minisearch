#ifndef MAKE_INVERTED_INDEX_H
#define MAKE_INVERTED_INDEX_H

#include <boost/noncopyable.hpp>
#include <vector>
#include "Document.h"
#include "InvertedIndex.h"

class MakeInvertedIndex : boost::noncopyable
{
public:
    void readPageLibIndex(); //读取网页库索引
    void readDocuments();  //根据索引，依次读取文章

    void computeFrequency(); //每篇文章计算词频
    void computeWeight(); //每篇文章计算权重
    void normalizeWordWeight(); //每篇文章进行归一化

    void addWeightToIndex(); //将每篇文档计算的权重放入倒排索引

    void saveToDisk(); //倒排索引保存
private:
    std::map<int, std::pair<long, size_t> > index_;
    InvertedIndex weightIndex_;  //单词权重的倒排索引
    std::vector<Document> documents_;
};

#endif //MAKE_INVERTED_INDEX_H