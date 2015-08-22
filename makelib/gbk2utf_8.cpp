#include "gbk2utf_8.h"
#include <cstddef>
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
int MY_LANGTRAN::CLang::gbk2utf8(char** utfStr,const char* srcStr)
{
    if(NULL == srcStr)
    {
        printf("bad Parameter:srcStr is null\n");
        return -1;
    }
    if(NULL == setlocale(LC_ALL,"zh_CN.gbk"))
    {
        printf("setlocale:bad Parameter\n");
        return -1;
    }
    int unicodeLen = mbstowcs(NULL,srcStr,0);
    if(unicodeLen<=0)
    {
        printf("mbstowcs:cannot transfer\n");
        return -1;
    }
    wchar_t* unicodeStr=(wchar_t*)calloc(sizeof(wchar_t),unicodeLen+1);
    if(unicodeStr == NULL)
    {
        printf("calloc failed\n");
        return -1;
    }
    mbstowcs(unicodeStr,srcStr,strlen(srcStr));
    if(NULL == setlocale(LC_ALL,"zh_CN.utf8"))
    {
        printf("setlocale:bad parameter\n");
        return -1;
    }
    int utfLen = wcstombs(NULL,unicodeStr,0);
    if(utfLen <=0)
    {
        printf("wcstombs:cannot transfer\n");
        free(unicodeStr);
        return -1;
    }
    *utfStr = (char*)calloc(sizeof(char),utfLen+1);
    if(*utfStr == NULL)
    {
        printf("calloc failed\n");
        free(unicodeStr);
        return -1;
    }
    wcstombs(*utfStr,unicodeStr,utfLen);
    (*utfStr)[utfLen]=0;
    free(unicodeStr);
    return utfLen;
}
void MY_LANGTRAN::CLang::destroy(char** utfStr)
{
    free(*utfStr);
    *utfStr = NULL;
}




