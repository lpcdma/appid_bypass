#include "stdafx.h"
#include "common.h"

VOID Split(const string& src, const string& separator, vector<string>& dest)
{
    string substring;
    string::size_type start = 0, index;

    do
    {
        index = src.find(separator, start);
        if (index != string::npos)
        {
            substring = src.substr(start, index - start);
            dest.push_back(substring);
            index += separator.length();
            if (index == src.length())
                return;
            start = index;
        }
    } while (index != string::npos);
    //the last token
    substring = src.substr(start);
    dest.push_back(substring);

    return;
}

string Trim(string &str)
{
    if (str.empty())
        return str;
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    return str;
}

BOOL Split(const string& src, const string& separator, string& strKey, string& strVal)
{
    string::size_type index;
    index = src.find(separator, 0);
    if (index == string::npos)
        return FALSE;
    strKey = src.substr(0, index);
    if (index + separator.length() == src.length())
        strVal = "";
    else
        strVal = src.substr(index + separator.length());
    return TRUE;
}

string makeRandString(int len)
{
    LARGE_INTEGER Tick;
    QueryPerformanceCounter(&Tick);
    srand(Tick.LowPart);

    string str_rand;
    for (int j = 0; j < len; j++)
    {
        int n = rand() % 16;
        char format[2] = { 0 };
        sprintf(format, "%x", n);
        str_rand += format;
    }

    return str_rand;
}