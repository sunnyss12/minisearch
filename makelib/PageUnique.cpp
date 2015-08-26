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

void PageUnique::computeWordFrequency()   //读pagelib，计算频率
{
    ReadFile rf("../data/pagelib.xml");
    if(!rf)
    {
        LOG_FATAL << "open pagelib failed.";
    }

    LOG_INFO << "开始读取pagelib,并计算词频";
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
        document.computeWordFrequency();  //边计算词频边删除conten，节省内存。之所以要获取全部词频才计算权重，是因为权重的计算需要一个词在多少文档出现，这需要把所有文档读完才能开始计算。
        document.clearContent();
        documents_.push_back(std::move(document));
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "pagelib读取完毕,词频计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";

}

void PageUnique::computeWordWeightTopK()   //计算权重，并获取前TopK的权重
{
    LOG_INFO << "开始计算词的权重";
    muduo::Timestamp beginTime(muduo::Timestamp::now());
    for(auto & d : documents_)
    {
        LOG_DEBUG << "docId" << d.getDocId();
        d.computeWordWeight();
        d.clearWordFrequency(); //清空词频
        d.extractTopK();
        d.clearWordWeight();
    }
    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "词权重计算完毕，花费" << muduo::timeDifference(endTime, beginTime) << " s";
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

        LOG_DEBUG << "docId: "<< docSimhash.first;

        key1 = docSimhash.second & 0xFFFF;
        simhashMap[key1].push_back(docSimhash);
        key2 = (docSimhash.second >> 16) & 0xFFFF;
        simhashMap[key2].push_back(docSimhash);
        key3 = (docSimhash.second >> 32) & 0xFFFF;
        simhashMap[key3].push_back(docSimhash);
        key4 = (docSimhash.second >> 48) & 0xFFFF;
        simhashMap[key4].push_back(docSimhash);
    }

    int hashsize = simhashMap.size();
    int hashnum = 0;

    bit_.resize(documents_.size(), true);
    for (auto &m : simhashMap)
    {
        hashnum++;
        LOG_DEBUG <<"共" << hashsize <<"个hash，现在是第" <<hashnum <<"个hash";
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
        LOG_DEBUG << "共" << bit_.size() <<"个文件需要写入,现在是第" <<ix <<"个文件";
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
