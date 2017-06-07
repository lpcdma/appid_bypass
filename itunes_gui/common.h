#pragma once
#include <Windows.h>
#include <string>
#include <vector>

using namespace std;


VOID Split(const string& src, const string& separator, vector<string>& dest);
string Trim(string &str);
BOOL Split(const string& src, const string& separator, string& strKey, string& strVal);
string makeRandString(int len);