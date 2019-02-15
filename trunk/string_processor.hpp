#ifndef STRING_PROCESSOR_H
#define STRING_PROCESSOR_H
#include <stdio.h>
#include <string.h>

using namespace std;
namespace Client
{
class StringProcessor
{
public:
    StringProcessor(){};
    ~StringProcessor(){};
    void RemoveSpaceOfHead(char *str)
    {
        if (NULL == str) return;
        unsigned int j=0, i=0;
        while (str[j] == ' ' || str[j] == '\t') ++j;
        for (i=0; i< strlen(str) - j; ++i) str[i] = str[i + j];
        str[i]='\0';
    }
    bool IsEmptyLine(char *str)
    {
        if (NULL == str) return true;
        unsigned int i=0;
        while (str[i] == ' ' || str[i] == '\t') ++i;
        if ('\0' == str[i]) return true;
        return false;
    }


};



}
#endif