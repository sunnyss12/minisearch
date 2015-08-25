#include "MakeInvertedIndex.h"
#include "ReadFile.h"
#include "SegmentSingleton.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
using namespace std;

void MakeInvertedIndex::readPageLibIndex()
{
    ReadFile rf("../data/page2.index");
    if(!rf)
    {
        LOG_FATAL << "open index file failed.";
    }
        
    LOG_INFO << "开始读取索引";
    muduo::Timestamp beginTime(muduo::Timestamp::now());


    int docNum = 0;

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
        auto ret = index_.insert(
            make_pair(docid, make_pair(offset, len)));
        assert(ret.second); (void)ret;

        docNum++;
    }

    //更新文档的数目
    SegmentSingleton::getInstance()->getNumOfDocument() = docNum;

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "读取索引完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
} 
void MakeInvertedIndex::readDocuments()
{
    ReadFile rf("../data/pagelib2.xml");
    if(!rf)
    {
        LOG_FATAL << "open pagelib failed.";
    }

    LOG_INFO << "开始读取pagelib";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    //遍历索引
    for(const auto &w : index_)
    {
        long offset = w.second.first;
        size_t len = w.second.second;
        rf.seekBeg(offset);
        string text = rf.readnBytesAsString(len);
        Document document;
        document.setText(std::move(text));
        documents_.push_back(std::move(document));
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "pagelib读取完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";

}  

void MakeInvertedIndex::computeFrequency()
{
    LOG_INFO << "开始计算词频";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto &d : documents_)
    {
        d.computeWordFrequency();
        //d.clearContent();  //不再需要正文部分
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "词频计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
} 
void MakeInvertedIndex::computeWeight()
{
    LOG_INFO << "开始计算权重";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto &d : documents_)
    {
        d.computeWordWeight();
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "权重计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
} 
void MakeInvertedIndex::normalizeWordWeight()
{
    LOG_INFO << "开始归一化";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto &d : documents_)
    {
        d.normalizeWordWeight();
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "归一化计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
} 

void MakeInvertedIndex::addWeightToIndex()
{
    LOG_INFO << "建立倒排索引";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    //addWeightToInvertedIndex
    for(const auto &d : documents_)
    {
        d.addWeightToInvertedIndex(weightIndex_);
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "建立倒排索引完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
}

void MakeInvertedIndex::saveToDisk()
{
    LOG_INFO << "保存倒排索引";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    weightIndex_.saveToDisk();

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "保存倒排索引完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
} 
