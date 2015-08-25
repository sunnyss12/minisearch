#include "PageUnique.h"
#include "ReadFile.h"
#include "TruncFile.h"
#include "SegmentSingleton.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <unordered_map>
using namespace std;

PageUnique::PageUnique(std::string indexPath)
: indexPath_(std::move(indexPath))
{

}

void PageUnique::readIndex()
{
    ReadFile rf("../data/page.index");
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

void PageUnique::readDocuments()
{
    ReadFile rf("../data/pagelib.xml");
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

void PageUnique::computeFrequency()
{
    LOG_INFO << "开始计算词频";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto &d : documents_)
    {
        d.computeWordFrequency();
        d.clearContent();  //不再需要正文部分
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "词频计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
}

void PageUnique::computeWordWeight()
{
    LOG_INFO << "开始计算词的权重";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto & d : documents_)
    {
        d.computeWordWeight();
    }
    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "词权重计算完毕，花费" << muduo::timeDifference(endTime, beginTime) << " s";
}

void PageUnique::computeTopK()
{
    LOG_INFO << "开始计算topK";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto &d : documents_)
    {
        d.extractTopK();
        d.clearWordFrequency(); //清空词频
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "topK计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
}

void PageUnique::unique()
{

    LOG_INFO << "开始网页去重";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    std::unordered_map<uint64_t,vector<pair<int,uint64_t> > > simhashMap; //key, docid, simhash
    pair<int,uint64_t> docSimhash;          //存储docId和simhash
    uint16_t key1,key2,key3,key4;
    for(auto& d: documents_)
    {
        docSimhash.first = d.getDocId();
        docSimhash.second = d.computeSimhash();

        key1 = docSimhash.second & 0xFFFF;
        simhashMap[key1].push_back(docSimhash);
        key2 = (docSimhash.second >> 16) & 0xFFFF;
        simhashMap[key2].push_back(docSimhash);
        key3 = (docSimhash.second >> 32) & 0xFFFF;
        simhashMap[key3].push_back(docSimhash);
        key4 = (docSimhash.second >> 48) & 0xFFFF;
        simhashMap[key4].push_back(docSimhash);
    }

    bit_.resize(documents_.size(), true);
    for (auto &m : simhashMap)
    {
        vector<pair<int,uint64_t> >& vec = m.second;
        for(size_t ix = 0; ix != vec.size()-1; ++ix)
        {
            if(bit_[vec[ix].first] == false)
                continue;
            //m[ix].first处的doc 与 [ix+1,size)
            for(size_t iy = ix+1; iy != vec.size(); ++iy)
            {
                if(bit_[vec[iy].first] == false)
                    continue;
                if(Simhash::Simhasher::isEqual(vec[ix].second,vec[iy].second))
                    bit_[vec[iy].first] = false;
            }
        }
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "网页去重完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";

}

void PageUnique::saveToPageLib()
{
    TruncFile indexfile("../data/page2.index");
    TruncFile pagelibFile("../data/pagelib2.xml");
    if(!indexfile || !pagelibFile)
    {
        LOG_FATAL << "save To Disk Error";
    }


    LOG_INFO << "开始写回磁盘，重新构建pagelib";
    muduo::Timestamp beginTime(muduo::Timestamp::now());

    for(size_t ix = 0; ix != bit_.size(); ++ix)
    {
        if(bit_[ix] == false)
            continue;

        char tmp[100] = {0};
        snprintf(tmp, sizeof tmp, "%d %ld %lu",
            documents_[ix].getDocId(),
            pagelibFile.seek(),
            documents_[ix].getText().size());
        indexfile.writeStringLine(tmp);
        pagelibFile.writeStringLine(documents_[ix].getText());
    }
    
    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "pagelib回写完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";

}
