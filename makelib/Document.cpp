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
        return w1.second < w2.second;
    }
};

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
    content_ = "";
}

void Document::clearWordFrequency()
{
    wordFrequency_.clear();
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
    LOG_INFO << "docid :" << docid_ << " words size : " << words.size();
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
    priority_queue<WordItem, vector<WordItem>, CompWordItem> topQueue;
    for(const WordItem &w : wordFrequency_)
    {
        topQueue.push(w);
    }

    LOG_DEBUG << "docid : " << docid_ << " topQueue size : " << topQueue.size();

    //取出topK的单词
    for(int i = 0; i != kTopWord; ++i)
    {
        if(!topQueue.empty())
        {
            WordItem w = topQueue.top();
            topQueue.pop();
            topK_.insert(std::move(w.first));
        }
    }

    LOG_DEBUG << "docid : " << docid_ << " topK_ size : " << topK_.size(); 
}

bool operator==(const Document &d1, const Document &d2)
{
    size_t len1 = d1.topK_.size();
    size_t len2 = d2.topK_.size();
    size_t len = (len1 < len2) ? len1 : len2;

    //求交集
    set<string> intersection;
    for(const string &s : d1.topK_)
    {
        if(d2.topK_.count(s))
            intersection.insert(s);
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

    module1 = sqrt(module1);   //因为w2中的权重本来都是做了归一化处理的了，不需要再计算归一化了,而且如果再对w1做归一化处理，会导致所有文章的similarity的值相同

    double member = 0.0;
    for(size_t ix = 0; ix != len; ++ix)
    {
        member += w1[ix]*w2[ix];
    }

    double similarity = member / module1 ;
   
    LOG_INFO << "docId:" << docid <<" similarity:" << similarity;
    return similarity;
}
