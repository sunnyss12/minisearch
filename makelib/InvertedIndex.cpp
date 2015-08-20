#include "InvertedIndex.h"
#include "TruncFile.h"
#include "ReadFile.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <sstream>
#include <limits>
using namespace std;

void InvertedIndex::addWeightItem(const std::string &word, int docid, double weight)
{
    auto ret = index_[word].insert(make_pair(docid, weight));
    //assert(ret.second);
    (void)ret;
}

InvertedIndex::WordWeight InvertedIndex::getWeightItem(const std::string &word) const
{
    auto iter = index_.find(word);
    if(iter == index_.end())
        return WordWeight();
    else
        return iter->second; //返回一个map
}

std::pair<double, bool> InvertedIndex::getWeight(const std::string &word, int docid) const
{
    auto iter = index_.find(word);
    if(iter == index_.end())
    {
        return make_pair(0.0, false);
    } 
    else
    {
        auto it = iter->second.find(docid);
        if(it == iter->second.end())
        {
            return make_pair(0.0, false);
        }
        else
        {
            return make_pair(it->second, true);
        }
    }
}

std::set<int> InvertedIndex::getDocIdSet(const std::string &word) const
{
    auto iter = index_.find(word);
    if(iter == index_.end())
        return std::set<int>();
    else
    {
        std::set<int> result;
        for(const auto &w : iter->second) //遍历map
        {
            result.insert(w.first);
        }
        return result;
    }
}

int InvertedIndex::getDfOfWord(const std::string &word) const
{
    auto iter = index_.find(word);
    if(iter == index_.end())
    {
        return 0;
    }
    else
    {
        return static_cast<int>(iter->second.size());
    }
}

//保存到磁盘上
void InvertedIndex::saveToDisk() const
{
    TruncFile tf("../data/inverted.index");
    if(!tf)
    {
        LOG_FATAL << "InvertedIndex saveToDisk failed.";
    }

    LOG_INFO << "Begin save InvertedIndex to Disk";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    //foo 12 0.004 34 0.034
    for(const auto &w : index_) // w: pair<string, map>
    {
        string item = w.first; //foo
        for(const auto &ww : w.second) // ww: pair<int, double>
        {
            char text[100] = {0};
            snprintf(text, sizeof text, "%d %lf", ww.first, ww.second);
            item.append(" ");
            item.append(text);
        }
        tf.writeStringLine(item);
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "save InvertedIndex Finish  cost : " << muduo::timeDifference(endTime, beginTime) << " s";
} 

//从磁盘加载
void InvertedIndex::loadFromDisk(const std::string &path)
{
    ReadFile rf(path);
    if(!rf)
    {
        LOG_FATAL << "open InvertedIndex failed.";
    }

    LOG_INFO << "开始读取倒排索引";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    string line;
    while((line = rf.readLineAsString()) != "")
    {
        istringstream is(line);
        string word;
        is >> word;
        int docid;
        double weight;
        while(is >> docid >> weight, !is.eof())
        {
            if(is.fail())  //处理错误输入
            {
                is.clear();  //重置状态
                is.ignore(std::numeric_limits < std::streamsize > ::max(), '\n');
                LOG_WARN << "InvertedIndex format error";
            }

            addWeightItem(word, docid, weight); //加入索引
        }
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "加载倒排索引完毕  cost : " << muduo::timeDifference(endTime, beginTime) << " s";
} 

