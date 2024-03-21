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
const int64_t ReadInt64Binary(FILE*& f);
const uint64_t ReadUint64Binary(FILE*& f);
void AppendInt64Binary(const int64_t number, FILE*& f);
void AppendUint64Binary(const uint64_t number, FILE*& f);
const int32_t ReadInt32Binary(FILE*& f);
const uint32_t ReadUint32Binary(FILE*& f);
void AppendInt32Binary(const int32_t number, FILE*& f);
void AppendUint32Binary(const uint32_t number, FILE*& f);
const int16_t ReadInt16Binary(FILE*& f);
const uint16_t ReadUint16Binary(FILE*& f);
void AppendInt16Binary(const int16_t number, FILE*& f);
void AppendUint16Binary(const uint16_t number, FILE*& f);
const int8_t ReadInt8Binary(FILE*& f);
const uint8_t ReadUint8Binary(FILE*& f);
void AppendInt8Binary(const int8_t number, FILE*& f);
void AppendUint8Binary(const uint8_t number, FILE*& f);


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
    if (!outFile.is_open()) {
        std::cout << "Error: File " << filePath << " doesn't exist" << std::endl;
        return;
    }

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

// read 64 bit int number from file in binary mode
const int64_t ReadInt64Binary(FILE*& f)
{
    int64_t number;
    fread(&number, sizeof(int64_t), 1, f);
    return number;
}

// read 64 bit unsigned int number from file in binary mode
const uint64_t ReadUint64Binary(FILE*& f)
{
    uint64_t number;
    fread(&number, sizeof(uint64_t), 1, f);
    return number;
}

// append 64 bit int number to file in binary mode
void AppendInt64Binary(const int64_t number, FILE*& f)
{
    fwrite(&number, sizeof(int64_t), 1, f);
}

// append 64 bit unsigned int number to file in binary mode
void AppendUint64Binary(const uint64_t number, FILE*& f)
{
    fwrite(&number, sizeof(uint64_t), 1, f);
}

// ==========================================================================================================

// read 32 bit int number from file in binary mode
const int32_t ReadInt32Binary(FILE*& f)
{
    int32_t number;
    fread(&number, sizeof(int32_t), 1, f);
    return number;
}

// read 32 bit unsigned int number from file in binary mode
const uint32_t ReadUint32Binary(FILE*& f)
{
    uint32_t number;
    fread(&number, sizeof(uint32_t), 1, f);
    return number;
}

// append 32 bit int number to file in binary mode
void AppendInt32Binary(const int32_t number, FILE*& f)
{
    fwrite(&number, sizeof(int32_t), 1, f);
}

// append 32 bit unsigned int number to file in binary mode
void AppendUint32Binary(const uint32_t number, FILE*& f)
{
    fwrite(&number, sizeof(uint32_t), 1, f);
}

// ==========================================================================================================

// read 16 bit int number from file in binary mode
const int16_t ReadInt16Binary(FILE*& f)
{
    int16_t number;
    fread(&number, sizeof(int16_t), 1, f);
    return number;
}

// read 16 bit unsigned int number from file in binary mode
const uint16_t ReadUint16Binary(FILE*& f)
{
    uint16_t number;
    fread(&number, sizeof(uint16_t), 1, f);
    return number;
}

// append 16 bit int number to file in binary mode
void AppendInt16Binary(const int16_t number, FILE*& f)
{
    fwrite(&number, sizeof(int16_t), 1, f);
}

// append 16 bit unsigned int number to file in binary mode
void AppendUint16Binary(const uint16_t number, FILE*& f)
{
    fwrite(&number, sizeof(uint16_t), 1, f);
}

// ==========================================================================================================

// read 8 bit int number from file in binary mode
const int8_t ReadInt8Binary(FILE*& f)
{
    int8_t number;
    fread(&number, sizeof(int8_t), 1, f);
    return number;
}

// read 8 bit unsigned int number from file in binary mode
const uint8_t ReadUint8Binary(FILE*& f)
{
    uint8_t number;
    fread(&number, sizeof(uint8_t), 1, f);
    return number;
}

// append 8 bit int number to file in binary mode
void AppendInt8Binary(const int8_t number, FILE*& f)
{
    fwrite(&number, sizeof(int8_t), 1, f);
}

// append 8 bit unsigned int number to file in binary mode
void AppendUint8Binary(const uint8_t number, FILE*& f)
{
    fwrite(&number, sizeof(uint8_t), 1, f);
}

// ==========================================================================================================