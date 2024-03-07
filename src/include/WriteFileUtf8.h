#pragma once

#include <fstream>

void WriteFileUtf8(const std::wstring& str, const std::string& outPath)
{
    /* FILE* outFile = fopen(outPath.c_str(), "w,ccs=UTF-8"); // Open for writing with UTF-8 encoding

    //auto wstr = str.c_str();

    fwrite(str.c_str(), 1, str.size() * sizeof(wchar_t), outFile);
    fclose(outFile); */

    std::ofstream outFile(outPath, std::ios::out | std::ios::binary);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8Str = converter.to_bytes(str);
    outFile.write(utf8Str.c_str(), utf8Str.size());
}
