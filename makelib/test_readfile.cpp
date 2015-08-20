#include "ReadFile.h"
#include <iostream>
#include <stdexcept>
using namespace std;

int main(int argc, char const *argv[])
{
    ReadFile rf("/home/wing/Documents/test.txt");
    if(!rf)
    {
        throw runtime_error("文件打开失败");
    }

    std::string line;
    while((line = rf.readLineAsString()) != "")
    {
        cout << line.size() << endl;
        //cout << line << endl;
    }

    cout << rf.seek() << endl;

    rf.seekBeg(0);

    const int kSize = 100;
    while((line = rf.readnBytesAsString(kSize)) != "")
    {
        cout << line.size() << endl;
    }

    rf.seekBeg(0);
    line = rf.readnBytesAsString(6000);
    cout << line.size() << endl;

    rf.seekBeg(0);
    line = rf.readnBytesAsString(10000);
    cout << line.size() << endl;

    return 0;
}