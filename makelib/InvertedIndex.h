#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <map>
#include <set>
#include <string>

class InvertedIndex : boost::noncopyable
{
public:
    typedef std::pair<int, double> WordWeightItem;
    typedef std::map<int, double> WordWeight; //单词的权重
    typedef std::unordered_map<std::string, WordWeight> WeightIndex;

    //增加某项
    void addWeightItem(const std::string &word, int docid, double weight);
    //重载两个查询的函数
    WordWeight getWeightItem(const std::string &word) const;
    std::pair<double, bool> getWeight(const std::string &word, int docid) const;
    std::set<int> getDocIdSet(const std::string &word) const;
    int getDfOfWord(const std::string &word) const;

    void saveToDisk() const; //保存至磁盘
    void loadFromDisk(const std::string &path); //从磁盘加载

private:
    //map<string, map<docid, weight> >
    WeightIndex index_;
};

#endif //INVERTED_INDEX_H