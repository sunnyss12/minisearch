#include "Document.h"
#include "SegmentSingleton.h"
#include "InvertedIndex.h"
#include <muduo/base/Logging.h>
#include <queue>
#include <math.h>
using namespace CppJieba;
using namespace std;

class CompWordItem //用于优先级队列
{
    typedef Document::WordItem WordItem;
    public:
    bool operator() (const WordItem &w1, const WordItem &w2) const
    {
        return w1.second > w2.second;
    }
};
Document::Document(Document&& doc)
{
    setDocId(doc.docid_);
    setTitle(doc.title_);
    setContent(doc.content_);
    text_ = std::move(doc.text_);
    wordFrequency_ = std::move(doc.wordFrequency_);
    topK_ = std::move(doc.topK_);
    wordWeight_ = std::move(doc.wordWeight_);
}

void Document::setDocId(int docid)
{
    docid_ = docid;
}

void Document::setTitle(std::string title)
{
    title_ = std::move(title);
}

void Document::setContent(std::string content)
{
    content_ = std::move(content);
}

void Document::setText(std::string text)
{
    //解析docid
    size_t docIdBegin = text.find("<docid>");
    size_t docIdEnd = text.find("</docid>");
    assert(docIdBegin != string::npos &&
            docIdEnd != string::npos);
    size_t tagLen = strlen("<docid>");
    string docid_ = text.substr(docIdBegin + tagLen, docIdEnd - docIdBegin - tagLen);
    int docid = ::atoi(docid_.c_str());
    setDocId(docid);

    //解析title
    size_t titleBegin = text.find("<title>");
    size_t titleEnd = text.find("</title>");
    assert(titleBegin != string::npos &&
            titleEnd != string::npos); 
    tagLen = strlen("<title>");
    string title = text.substr(titleBegin + tagLen, titleEnd - titleBegin - tagLen);
    setTitle(title);

    //解析content
    size_t contentBegin = text.find("<content>");
    size_t contentEnd = text.find("</content>");
    assert(contentBegin != string::npos &&
            contentEnd != string::npos); 
    tagLen = strlen("<content>");
    string content = text.substr(contentBegin + tagLen, contentEnd - contentBegin - tagLen);
    setContent(content);

    //
    text_ = std::move(text);
}

void Document::clearContent()
{
    string().swap(content_);
}

void Document::clearWordFrequency()
{
    map<string,int>().swap(wordFrequency_);
    //map<string,int> tmp(std::move(wordFrequency_));
}

void Document::clearWordWeight()
{
    map<string,double>().swap(wordWeight_);
    //map<string,double> tmp(std::move(wordWeight_));
}

//计算词频
void Document::computeWordFrequency()
{
    //通过单例模式 获取分词器
    SegmentSingleton *singleton = SegmentSingleton::getInstance();
    const Application &segment = singleton->getSegment();
    const set<string> &stopList = singleton->getStopList();
    vector<string> words;

    std::unordered_map<std::string, int> &dfOfDocument = singleton->getDfOfDocument();  //全局DF,全局词与共有多少文档含有该词

    //分词
    segment.cut(content_, words);
    LOG_DEBUG << "docid :" << docid_ << " words size : " << words.size();
    //统计词频
    for(const string &w : words)
    {
        if(!stopList.count(w)) //不是停用词
        {
            wordFrequency_[w]++; //自身的词与词频
        }
    }

    for(const auto &w : wordFrequency_)
    {
        dfOfDocument[w.first]++;  //词和有多少文档出现该词
    }
}

//计算topK
void Document::extractTopK()
{
    priority_queue<WordItem, vector<WordItem>, CompWordItem> topQueue;  //最小堆
    for(const WordItem &w : wordWeight_)
    {
        if(topQueue.size() < kTopWord)
            topQueue.push(w);
        else
        {
            if(topQueue.top().second < w.second)
            {
                topQueue.pop();
                topQueue.push(w);
            }
        }
    }
    while(topQueue.size()>0)   //有可能topQueue的size小于kTopWord，所以显示topQueue的size不能用kTopWord比较
    {
        topK_.push_back(topQueue.top());
        topQueue.pop();
    }
}

uint64_t Document::computeSimhash()
{
    return simhasher_.make(topK_);
}
bool operator==(const Document &d1, const Document &d2)
{
    size_t len1 = d1.topK_.size();
    size_t len2 = d2.topK_.size();
    size_t len = (len1 < len2) ? len1 : len2;

    //求交集
    set<string> intersection;
    for(const auto& w1 : d1.topK_)
    {
        for(const auto& w2 : d2.topK_)
        {
            if(w1.first == w2.first)
                intersection.insert(w1.first);
        }
    }

    size_t len_intersection = intersection.size();

    LOG_DEBUG << "docid : " << d1.getDocId() << " docid : " << d2.getDocId() << " intersection size " << len_intersection;

    if(static_cast<double>(len_intersection) / len > 0.85)
        return true;
    return false;
}

bool operator!=(const Document &d1, const Document &d2)
{
    return !(d1 == d2);
}

//计算单词的权重
void Document::computeWordWeight()
{
    //获取全局词频
    const std::unordered_map<std::string, int> &dfOfDocument = SegmentSingleton::getInstance()->getDfOfDocument();
    int N = SegmentSingleton::getInstance()->getNumOfDocument();
    assert(N != 0); //不要忘记更新文档数目
    //计算权重
    for(const WordItem &w : wordFrequency_)
    {
        auto iter = dfOfDocument.find(w.first);
        assert(iter != dfOfDocument.end());
        assert(iter->second > 0);

        int tf = w.second;     //词的频率
        int df = iter->second; //df:词在多少文档出现过
        assert(N >= df);
        double weight = tf * log(N / static_cast<double>(df));
        assert(weight >= 0);
        //wordHeight_[w.first] = weight;
        auto ret = wordWeight_.insert(make_pair(w.first, weight));  //词和权重
        assert(ret.second); (void)ret;

        //printf("[ %d %d %lf ]", tf, df, weight);
    }
}

//将文档内每个单词的权重归一化
void Document::normalizeWordWeight()
{
    double total = 0.0;
    for(const auto &w : wordWeight_) //pair<string, double>
    {
        total += (w.second*w.second);
    }
    total = ::sqrt(total);

    for(auto &w : wordWeight_) //pair<string, double>
    {
        double normalizeWeight = w.second / total;
        w.second = normalizeWeight;
    }
}

void Document::addWeightToInvertedIndex(InvertedIndex &index) const
{
    for(auto &w : wordWeight_) //pair<string, double>
    {
        index.addWeightItem(w.first, docid_, w.second);  //词，docid,词权重
    }
}

double Document::computeSimilarity(int docid, const std::vector<std::pair<std::string, double>> &vec, const InvertedIndex &index)
{
    vector<double> w1; //将vec中的double抽取出来,表示所有查找词的权重
    vector<double> w2; //根据本doc生成的向量，表示所有查找词在文档为docid的权重

    for(const auto &w : vec)
    {
        w1.push_back(w.second);
        auto ret = index.getWeight(w.first, docid);
        if(ret.second == false)
            w2.push_back(0.0);
        else
            w2.push_back(ret.first);
    }


    //利用余弦模型
    assert(w1.size() == w2.size());
    size_t len = w1.size();
    double module1 = 0.0;
    for(const auto &w : w1)
    {
        module1 += (w*w);
    }

    module1 = sqrt(module1);   
    double member = 0.0;
    for(size_t ix = 0; ix != len; ++ix)
    {
        member += w1[ix]*w2[ix];
    }

    double similarity = member / module1 ;   //原版本中还计算了module2，要对w2做归一化处理。计算similarity时除以了module2。这是错误的。因为w2中的权重本来都是做了归一化处理的了，不需要再计算归一化了；而且如果非要像上版本那样做归一化处理，其实是错误的。验证方法：假设输入两个查询词，两个查询词的权重向量为<a1,a2>,并且第二个查询词在所有文档中都不存在，假设文档A的两个查询词权重为<b1,0>,文档B的两个查询词权重为<c1,0>，那么如果再对<b1,0>和<c1,0>做归一化处理，那么文档A的相似度为a1*b1/sqrt(a1*a1+a2*a2)/b1=a1/sqrt(a1*a2+a2*a2)，文档B的相似度也是这个值，显然是不对的。


    LOG_DEBUG << "docId:" << docid <<" similarity:" << similarity;
    return similarity;
}
