#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <locale>

// read all characters (wide characters) from file to std::wstring
std::wstring ReadWideContent(const std::string& filePath)
{
    std::wifstream wif(filePath.c_str());
    if (!wif.is_open()) {
        std::cout << "Error: File " << filePath << " doesn't exist" << std::endl;
        return L"";
    }

    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    wif.close();

    return wss.str();
}

const wchar_t ReadWideCharBinary(std::wifstream& file)
{
    wchar_t c;
    file.read(reinterpret_cast<wchar_t*>(&c), sizeof(c));
}

const int ReadNumberBinary(std::wifstream& file)
{
    int number;
    file.read(reinterpret_cast<wchar_t*>(&number), sizeof(number));
}

// write std::wstring to file
void WriteWideContent(const std::wstring& str, const std::string& filePath)
{
    std::ofstream outFile(filePath);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8Str = converter.to_bytes(str);
    outFile.write(utf8Str.c_str(), utf8Str.size());

    /* std::wofstream outFile(filePath, std::ios::out);
    outFile.write(str.c_str(), str.size() * sizeof(wchar_t)); */
}

// write std::wstring to file in binary mode
void WriteWideStrBinary(const std::wstring& str, const std::string& filePath)
{
    std::wofstream outFile(filePath, std::ios::out | std::ios::binary);
    for (wchar_t c : str) {
        outFile.write(reinterpret_cast<const wchar_t*>(c), sizeof(wchar_t));
    }
}

// append character (wide character) to file
void AppendWideCharBinary(const wchar_t c, const std::string& filePath)
{
    std::wofstream outFile(filePath, std::ios::app | std::ios::binary);
    outFile.write(reinterpret_cast<const wchar_t*>(c), sizeof(wchar_t));
}

// append number to file
void AppendNumberBinary(const int number, const std::string& filePath)
{
    std::ofstream outFile(filePath, std::ios::app | std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(&number), sizeof(number));
}