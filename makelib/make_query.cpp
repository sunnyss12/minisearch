#include "PageQuery.h"
#include "SegmentSingleton.h"
#include <muduo/base/Logging.h>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{
    muduo::Logger::setLogLevel(muduo::Logger::DEBUG);

    SegmentSingleton::getInstance();

    PageQuery pageQuery;

    pageQuery.readPageLibIndex();
    pageQuery.readInvertedIndex();

    string word;
    string s;
    while(getline(cin, word))
    {
        s = pageQuery.queryPage(word);
        std::cout<<s;
    }

    return 0;
}
