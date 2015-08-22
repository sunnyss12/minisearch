#ifndef __GBK2UTF_8_H__
#define __GBK2UTF_8_H__
namespace MY_LANGTRAN
{
    class CLang
    {
        public:
            int gbk2utf8(char** utfStr,const char* srcStr);
            void destroy(char** utfStr);
    };
}
#endif


