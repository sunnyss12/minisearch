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
        if(sscanf(item.c_str(), "%d %ld %lu", &docid, &offset, &len) != 3)
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
//求查询词和各篇文档的相似度，原理：把查询词也看成一篇文档，那么计算查询词文档的权重，计算方法与一般文档的权重计算相同，然后利用余弦定理计算查询词权重向量与文档权重向量的夹角，夹角越小，相似度越高。计算夹角时。在计算查询词与每篇文章的相似度时，原理上应该先求查询词与文章关键词的交集，然后再计算这些交集查询查向量的余弦角。但鉴于查询词数量一般比较少，可以直接计算查询词权重向量与各篇文章查询词组成的权重向量的余弦。
std::string PageQuery::queryPage(std::string &word)const    
{
    //分词
    SegmentSingleton *singleton = SegmentSingleton::getInstance();
    const Application &segment = singleton->getSegment();
    const set<string> &stopList = singleton->getStopList();
    string str_upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    size_t pos = 0;
    while( (pos = word.find_first_of(str_upper,pos)) != std::string::npos)
    {
        word[pos] = word[pos] + 'a' - 'A';
        pos++;
    }
    vector<string> words;
    segment.cut(word, words);
    unordered_map<string,int> queryMap;
    for(const string & w : words)
    {
        if(!stopList.count(w))
        {
            queryMap[w]++;
        }
    }
    bool singleWord = (queryMap.size() == 1); //是否为单个查询词，如果是单个查询词，则可以简化相似度的计算
    string single_Word = queryMap.begin()->first;

    //计算TF_IDF生成向量
    vector<pair<string, double>> weightVec; //所有的查询词和权重组成的查询词向量
    for(const auto &w : queryMap)
    {
        int df = wordWeightIndex_.getDfOfWord(w.first);
        int N = pageLibIndex_.size();
        assert(N > 0);
        double weight = 0.0;  
        if(df == 0)
            weight = 0;  //当df=0，设置weight=0的原因是：(1)N/df不能计算(2)计算相似性其实是查询词权重向量与查询词在各篇文章权重组成的向量的余弦计算，既然某文档中该查询词向量分量为0，那么把查询词的向量分量设置为0也没关系
        else
            weight = w.second * log(static_cast<double>(N) / df);
        weightVec.push_back(make_pair(w.first, weight));
    }

    //查找docset
    set<int> docSet;   //把查找词所在的docId都放到docSet
    for(const auto &w : weightVec)
    {
        set<int> s = wordWeightIndex_.getDocIdSet(w.first);
        docSet.insert(s.begin(), s.end());
    }


    //计算相似度 docid -> sim
    vector<pair<int, double>> similarityResult; //相似度计算结果:docId和相似度
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
            similarity = Document::computeSimilarity(w, weightVec, wordWeightIndex_);  //计算查找词与文档id为w的相似度: w为docId，weightVec为查找词各词的权重,wordWeightIndex为所有倒排索引,即inverted.index的内容
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
