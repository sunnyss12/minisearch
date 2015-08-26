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
    void computeWordFrequency();  //根据索引，依次读取文章,并对每篇文章分词，然后计算每篇文章分词的词频
    void computeWordWeightIndex(); //计算每篇文章的分词和词频计算权重，并计算归一化处理，并把归一后的词频都放到倒排索引
    void saveToDisk(); //倒排索引保存
private:
    std::map<int, std::pair<long, size_t> > index_;
    InvertedIndex weightIndex_;  //单词权重的倒排索引
    std::vector<Document> documents_;
};

#endif //MAKE_INVERTED_INDEX_H
