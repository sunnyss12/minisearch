#include "PageUnique.h"
#include "SegmentSingleton.h"
#include <muduo/base/Logging.h>
using namespace muduo;

class InitSegment
{
public:
    InitSegment()
    {
        SegmentSingleton::getInstance(); 
    }
};

InitSegment initSegment;


int main(int argc, char const *argv[])
{
    Logger::setLogLevel(Logger::DEBUG);

    PageUnique pageUnique("../data/page.index");

    pageUnique.readIndex();
    pageUnique.computeWordFrequency();
    pageUnique.computeWordWeightTopK();
    pageUnique.unique();
    pageUnique.saveToPageLib();

    return 0;
}
