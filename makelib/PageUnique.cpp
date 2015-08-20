#include "PageUnique.h"
#include "ReadFile.h"
#include "TruncFile.h"
#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
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
        auto ret = index_.insert(
            make_pair(docid, make_pair(offset, len)));
        assert(ret.second); (void)ret;
    }

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
        //d.clearContent();  //不再需要正文部分
    }

    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "词频计算完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";
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

    bit_.resize(documents_.size(), true);

    for(size_t ix = 0; ix != documents_.size()-1; ++ix)
    {
        if(bit_[ix] == false)
            continue;
        //ix位置处的doc 与 [ix+1, size)
        for(size_t iy = ix+1; iy != documents_.size(); ++iy)
        {
            if(bit_[iy] == false)
                continue; //网页已经被删除，则不必比较
            if(documents_[ix] == documents_[iy])
                bit_[iy] = false;
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
        snprintf(tmp, sizeof tmp, "%d %ld %u",
            documents_[ix].getDocId(),
            pagelibFile.seek(),
            documents_[ix].getText().size());
        indexfile.writeStringLine(tmp);
        pagelibFile.writeStringLine(documents_[ix].getText());
    }
    
    muduo::Timestamp endTime(muduo::Timestamp::now());
    LOG_INFO << "pagelib回写完毕，花费 " << muduo::timeDifference(endTime, beginTime) << " s";

}