#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <locale>

// list of all functions for working with files
const std::wstring ReadWideContent(const std::string& filePath);
void WriteWideContent(const std::wstring& str, const std::string& filePath);
const std::wstring ReadWideStrBinary(FILE*& f, const size_t size);
void AppendWideStrBinary(const std::wstring& str, FILE*& f);
const wchar_t ReadWideCharBinary(FILE*& f);
void AppendWideCharBinary(const wchar_t c, FILE*& f);
const int ReadIntBinary(FILE*& f);
void AppendIntBinary(const int number, FILE*& f);


// read all characters (wide characters) from file to std::wstring
const std::wstring ReadWideContent(const std::string& filePath)
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

    // PREVIOUS IMPLEMENTATION
    /* FILE* file = fopen(filePath.c_str(), "r");
    wchar_t buffer[100];
    std::wstring result = L"";

    if (file) {
        while (fgetws(buffer, 100, file)) {
            result.append(buffer);
        }

        fclose(file);
    } else {
        std::cout << "Error: File " << filePath << " doesn't exist" << std::endl;
    }

    return result; */
}

// write std::wstring to file
void WriteWideContent(const std::wstring& str, const std::string& filePath)
{
    std::ofstream outFile(filePath);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8Str = converter.to_bytes(str);
    outFile.write(utf8Str.c_str(), utf8Str.size());

    // PREVIOUS IMPLEMENTATIONS
    /* FILE* file = fopen(filePath.c_str(), "w");
    if (file) {
        fputws(str.c_str(), file);
        
        fclose(file);
    } else {
        std::cout << "Error: File " << filePath << " doesn't exist" << std::endl;
    } */
    
    /* std::wofstream outFile(filePath, std::ios::out);
    outFile.write(str.c_str(), str.size() * sizeof(wchar_t)); */
}

// ==========================================================================================================

// read std::wstring from file in binary mode
const std::wstring ReadWideStrBinary(FILE*& f, const size_t size)
{
    std::wstring str;
    fread(&str, sizeof(wchar_t), size, f);
    return str;
}

// write std::wstring to file in binary mode
void AppendWideStrBinary(const std::wstring& str, FILE*& f)
{
    //fwrite(&str, sizeof(wchar_t), str.size(), f);
    for (int i = 0; i < str.size(); ++i) {
        AppendWideCharBinary(str[i], f);
    }
}

// ==========================================================================================================

// read character (wide character) from file in binary mode
const wchar_t ReadWideCharBinary(FILE*& f)
{
    wchar_t c;
    fread(&c, sizeof(wchar_t), 1, f);
    return c;
}

// append character (wide character) to file in binary mode
void AppendWideCharBinary(const wchar_t c, FILE*& f)
{
    fwrite(&c, sizeof(wchar_t), 1, f);
}

// ==========================================================================================================

// read number from file in binary mode
const int ReadIntBinary(FILE*& f)
{
    int number;
    fread(&number, sizeof(int), 1, f);
    return number;
}

// append number to file in binary mode
void AppendIntBinary(const int number, FILE*& f)
{
    fwrite(&number, sizeof(int), 1, f);
}

// ==========================================================================================================