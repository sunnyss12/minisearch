#ifndef PAGE_QUERY_H
#define PAGE_QUERY_H

#include <boost/noncopyable.hpp>
#include <string>
#include <map>
#include <vector>
#include "InvertedIndex.h"
#include "Document.h"

class PageQuery : boost::noncopyable
{
public:
    void readPageLibIndex(); //读取网页库索引
    void readInvertedIndex(); //读取倒排索引

    std::string queryPage(const std::string &word) const; //查询网页

    std::string getJsonResult(const std::vector<std::pair<int, double>> &result) const;
    Document getDocumentById(int docid) const;
private:
    std::map<int, std::pair<long, size_t> > pageLibIndex_; //网页索引
    InvertedIndex wordWeightIndex_;  //单词权重的倒排索引
};

#endif //PAGE_QUERY_H