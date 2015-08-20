#include "PageQuery.h"
#include "SegmentSingleton.h"
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char const *argv[])
{

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
