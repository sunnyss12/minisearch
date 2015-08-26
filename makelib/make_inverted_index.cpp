#include "MakeInvertedIndex.h"
#include <muduo/base/Logging.h>

int main(int argc, char const *argv[])
{
    muduo::Logger::setLogLevel(muduo::Logger::DEBUG);
    
    MakeInvertedIndex foo;
    foo.readPageLibIndex();
    foo.computeWordFrequency();
    foo.computeWordWeightIndex();
    foo.saveToDisk();

    return 0;
}
