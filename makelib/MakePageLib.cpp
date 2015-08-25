#include "MakePageLib.h"
#include "ReadFile.h"
#include "gbk2utf_8.h"
#include <muduo/base/Logging.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

namespace 
{
    //int转化为string
    std::string integerToString(int num)
    {
        char tmp[100] = {0};
        snprintf(tmp, sizeof tmp, "%d", num);
        return tmp;
    }
}

MakePageLib::MakePageLib(std::string dataDirectory,
        const std::string &pagelib,
        const std::string &indexfile)
: docid_(0),
    dataDirectory_(std::move(dataDirectory)),
    pagelib_(pagelib),
    index_(indexfile)
{

}

void MakePageLib::traversDirectoryWithRecursion(const std::string &path)
{
    DIR *dir = opendir(path.c_str());
    if(dir == NULL)
    {
        LOG_FATAL << "open dir error:" << path;  
    }
    chdir(path.c_str()); //更改工作目录

    struct dirent *pd;
    while((pd = readdir(dir)) != NULL)
    {
        if(pd->d_name[0] == '.') // . ..
            continue;

        //int lstat(const char *path, struct stat *buf);
        struct stat buf;
        if(lstat(pd->d_name, &buf) == -1)
        {
            LOG_ERROR << "lstat file :" << pd->d_name << " error";
            continue;
        }

        if(S_ISDIR(buf.st_mode))
            traversDirectoryWithRecursion(pd->d_name);
        else if(S_ISREG(buf.st_mode))
            processFile(pd->d_name);

    }

    closedir(dir);
    chdir(".."); //退出到上一层
}

void MakePageLib::traversDirectory()
{
    //缓存工作路径
    char tmp[1024] = {0};
    if(getcwd(tmp, sizeof tmp) == NULL)
        LOG_FATAL << "getcwd error";

    LOG_INFO << "PageLib Begin Read dataDirectory";

    traversDirectoryWithRecursion(dataDirectory_);

    LOG_INFO << "PageLib Finish Read dataDirectory";

    chdir(tmp);
}

void MakePageLib::processFile(const std::string &filename)
{
    LOG_INFO << "PageLib Process File : " << filename;

    ReadFile readFile(filename);
    std::string doc = "";
    doc += "<doc>";

    int docid = docid_++;
    doc += "<docid>";
    doc += integerToString(docid);
    doc += "</docid>";

    std::string title;
    MY_LANGTRAN::CLang lang;
    char* utfStr = NULL;
    int ret = -1;
    do
    {
        title = readFile.readLineAsString();
        if(title == "")
            return;
        ret = lang.gbk2utf8(&utfStr,title.c_str());
    }while(title == "\r\n" || title == "\n" || ret == -1);
    doc += "<title>";
    title = utfStr;
    lang.destroy(&utfStr);
    doc += title;
    doc += "</title>";

    std::string content;
    std::string line;
    std::string str_upper("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    size_t pos;
    while((line = readFile.readLineAsString()) != "")
    {
        if(line == "\r\n" || line == "\n")
            continue;
        ret = lang.gbk2utf8(&utfStr,line.c_str());
        if(ret == -1)
            continue;
        line = utfStr;
        lang.destroy(&utfStr);
        pos = 0;
        while( (pos = line.find_first_of(str_upper,pos)) != std::string::npos)
        {
            line[pos] = line[pos] + 'a' - 'A';
            pos++;
        }
        content += line;
        //content += "\n";
    }


    doc += "<content>";
    doc += content;
    doc += "</content>";

    doc += "</doc>";

    //写入偏移信息  docid offset length
    char text[100] = {0};
    snprintf(text, sizeof text, "%d %ld %lu", docid, pagelib_.seek(), doc.size());
    index_.writeStringLine(text);

    //写入pagelib
    pagelib_.writeStringLine(doc);


    //这里为了测试去重，可以写入两次，但是docid不能重复
}
