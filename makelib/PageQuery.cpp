#include "PageQuery.h"
#include "ReadFile.h"
#include "SegmentSingleton.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <algorithm>
#include <json/json.h>
using namespace std;
using namespace CppJieba;

void PageQuery::readPageLibIndex()
{
    ReadFile rf("../data/page2.index");
    if(!rf)
    {
        LOG_FATAL << "open index file failed.";
    }
        
    LOG_INFO << "开始读取索引";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    string item;
    int docid;
    long offset;
    size_t len;
    while((item = rf.readLineAsString()) != "")
    {
        //%d %ld %u
        if(sscanf(item.c_str(), "%d %ld %u", &docid, &offset, &len) != 3)
        {
            LOG_ERROR << "index format error";
        }
        auto ret = pageLibIndex_.insert(
            make_pair(docid, make_pair(offset, len)));
        assert(ret.second); (void)ret;

    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "读取索引完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
}

void PageQuery::readInvertedIndex()
{
    wordWeightIndex_.loadFromDisk("../data/inverted.index");
}

namespace 
{
bool cmpSimilarity(const pair<int, double> &p1, const pair<int, double> &p2)
{
    if(p1.second != p2.second)
        return p1.second > p2.second;
    else
        return p1.first < p2.first;
}
}

std::string PageQuery::queryPage(const std::string &word) const
{
    //分词
    SegmentSingleton *singleton = SegmentSingleton::getInstance();
    const Application &segment = singleton->getSegment();
    const set<string> &stopList = singleton->getStopList();
    vector<string> words;
    segment.cut(word, words);
    set<string> querySets(words.begin(), words.end()); //自动去重

    bool singleWord = (querySets.size() == 1); //是否为单个查询词
    string single_Word = *querySets.begin();

    //计算TF_IDF生成向量
    vector<pair<string, double>> weightVec;
    for(const auto &w : querySets)
    {
        if(!stopList.count(w))
        {
            int df = wordWeightIndex_.getDfOfWord(w);
            int N = pageLibIndex_.size();
            assert(N > 0);
            double weight = 0.0;
            if(df == 0)
                weight = 0;
            else
                weight = 1 * log(static_cast<double>(N) / df);
            weightVec.push_back(make_pair(w, weight));
        }
    }

    //查找docset
    set<int> docSet;
    for(const auto &w : weightVec)
    {
        set<int> s = wordWeightIndex_.getDocIdSet(w.first);
        docSet.insert(s.begin(), s.end());
    }

    //计算相似度 docid -> sim
    vector<pair<int, double>> similarityResult; //相似度计算结果
    for(const auto &w : docSet)
    {
        double similarity = 0.0;
        if(singleWord)
        {
            auto ret = wordWeightIndex_.getWeight(single_Word, w);
            if(ret.second == false)
                similarity = 0.0;
            else
                similarity = ret.first;
        }
        else
        {
            similarity = Document::computeSimilarity(w, weightVec, wordWeightIndex_);
        }
         
        similarityResult.push_back(make_pair(w, similarity));
    }

    //排序
    std::sort(similarityResult.begin(), similarityResult.end(), cmpSimilarity);

    //打印
    return getJsonResult(similarityResult);
}

std::string PageQuery::getJsonResult(const vector<pair<int, double>> &similarityResult) const
{
    Json::Value root;
    Json::Value array;
    for(const auto &w : similarityResult)
    {
        Document d = getDocumentById(w.first);
        string title = d.getTitle();
        //string content = d.getContent();
        Json::Value item;
        item["title"] = title;
        array.append(item);
    }
    root["docs"] = array;

    return root.toStyledString();
}

Document PageQuery::getDocumentById(int docid) const
{
    //pageLibIndex_
    auto iter = pageLibIndex_.find(docid);
    assert(iter != pageLibIndex_.end());
    long offset = iter->second.first;
    size_t len = iter->second.second;

    ReadFile rf("../data/pagelib2.xml");
    if(!rf)
    {
        LOG_FATAL << "获取Document失败";
    }
    rf.seekBeg(offset);
    string text = rf.readnBytesAsString(len);

    Document result;
    result.setText(text);

    return result;
}
