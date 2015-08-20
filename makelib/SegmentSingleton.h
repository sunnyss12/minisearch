#ifndef SEGMENT_SINGLETON_H
#define SEGMENT_SINGLETON_H

#include <boost/noncopyable.hpp>
#include <muduo/base/Mutex.h>
#include <unordered_map>
#include "../include/libjieba/MixSegment.hpp"

//这是一个单例类，用于从项目中获取结巴分词的实例

class SegmentSingleton : boost::noncopyable
{
public:
    static SegmentSingleton *getInstance();
    const CppJieba::MixSegment &getSegment() const;
    const std::set<std::string> &getStopList() const;
    std::unordered_map<std::string, int> &getDfOfDocument();
    int &getNumOfDocument();
private:
    SegmentSingleton();
    void readStopList();
    CppJieba::MixSegment segment_; //分词器
    std::set<std::string> stopList_; //停用词

    std::unordered_map<std::string, int> dfOfDocument_; //全局词频
    int numOfDocuments_; //文档的数目

    static SegmentSingleton *pInstance_;
    static muduo::MutexLock mutex_;
};


#endif //SEGMENT_SINGLETON_H