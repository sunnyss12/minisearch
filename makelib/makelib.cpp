#include <string>
#include <muduo/base/Logging.h>
#include "MakePageLib.h"

int main(int argc, char const *argv[])
{
    MakePageLib pagelib(
           "/home/sun/src/1230_text_query/data/复旦文本分类测试语料",
            "../data/pagelib.xml",
            "../data/page.index");

    pagelib.traversDirectory();

    return 0;
}


