#include "SegmentSingleton.h"
#include "ReadFile.h"
#include <muduo/base/Logging.h>
using namespace CppJieba;

SegmentSingleton *SegmentSingleton::pInstance_ = NULL;
muduo::MutexLock SegmentSingleton::mutex_;

SegmentSingleton::SegmentSingleton()
: segment_("../data/jieba.dict.utf8", "../data/hmm_model.utf8")
{

}

SegmentSingleton *SegmentSingleton::getInstance()
{
    if(pInstance_ == NULL)
    {
        //加锁
        muduo::MutexLockGuard scopeLock(mutex_);
        if(pInstance_ == NULL)
        {
            LOG_INFO << "SegmentSingleton generate a instance";
            pInstance_ = new SegmentSingleton;
            pInstance_->readStopList();
        }
    }
    return pInstance_;
}

const MixSegment &SegmentSingleton::getSegment() const
{
    return segment_;
}

const std::set<std::string> &SegmentSingleton::getStopList() const
{
    return stopList_;
}

void SegmentSingleton::readStopList()
{
    ReadFile readFile("../data/stopList.txt");
    if(!readFile)
    {
        LOG_FATAL << "stopList open failed.";
    }

    string s;
    while((s = readFile.readLineAsString()) != "")
    {
        if(s[s.size()-1] == '\n')
        {
            s.erase(s.size()-1);
        }
            
        stopList_.insert(s);
    }
        

    LOG_INFO << "read StopList Finish.";
}

std::unordered_map<std::string, int> &SegmentSingleton::getDfOfDocument()
{
    return dfOfDocument_;
}

int &SegmentSingleton::getNumOfDocument()
{
    return numOfDocuments_;
}
