#include "TruncFile.h"
#include <string>
#include <stdexcept>
using namespace std;

int main(int argc, char const *argv[])
{
    TruncFile tf("text.txt");
    if(!tf)
        throw runtime_error("文件打开失败");

    tf.writeStringLine("hello");
    tf.writeStringLine("hello");
    tf.writeStringLine("hello");
    tf.writeStringLine("hello");
    tf.writeStringLine("hello");
    return 0;
}
