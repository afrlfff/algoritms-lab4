#pragma once

#include <sstream>
#include <codecvt>
#include <locale>

#include "WriteFileUtf8.h"

std::wstring ReadFileUtf8(const std::string& filePath)
{
    std::wifstream wif(filePath.c_str());
    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    wif.close();

    return wss.str();
}