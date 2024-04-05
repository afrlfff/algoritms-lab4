#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
// for correct wide character reading
#include <codecvt>
#include <locale>

class FileUtils
{
private:
    FileUtils() = default;
public:
    static std::ifstream OpenRead(const char* filepath);
    static std::wifstream OpenReadWide(const char* filepath);
    static std::ofstream OpenWrite(const char* filepath);
    static std::wofstream OpenWriteWide(const char* filepath);
    static FILE* OpenReadBinary(const char* filepath);
    static FILE* OpenWriteBinary(const char* filepath);

    static void CloseFile(std::ifstream& file);
    static void CloseFile(std::ofstream& file);
    static void CloseFile(std::wifstream& file);
    static void CloseFile(std::wofstream& file);
    static void CloseFile(FILE* file);

    static bool ContainsWideChars(const char* filepath);

    static const std::string ReadContent(const char* filepath);
    static const std::wstring ReadWideContent(const char* filepath);
    static void WriteContent(const char* filepath, const std::string& content);
    static void WriteWideContent(const char* filepath, const std::wstring& content);

    static void AppendChar(std::ofstream& file, const char c);
    static void AppendWideChar(std::wofstream& file, const wchar_t c);
    static void AppendStr(std::ofstream& file, const std::string& str);
    static void AppendWideStr(std::wofstream& file, const std::wstring& str);

    static const char ReadCharBinary(FILE* file);
    static const wchar_t ReadWideCharBinary(FILE* file);
    static void AppendCharBinary(FILE* file, const char& c);
    static void AppendWideCharBinary(FILE* file, const wchar_t& c);
    static const std::string ReadStrBinary(FILE* file, const size_t& size);
    static const std::wstring ReadWideStrBinary(FILE* file, const size_t& size);
    static void AppendStrBinary(FILE* file, const std::string& str);
    static void AppendWideStrBinary(FILE* file, const std::wstring& str);

    static const int64_t ReadInt64Binary(FILE* file);
    static const uint64_t ReadUint64Binary(FILE* file);
    static void AppendInt64Binary(FILE* file, const int64_t& number);
    static void AppendUint64Binary(FILE* file, const uint64_t& number);
    static const int32_t ReadInt32Binary(FILE* file);
    static const uint32_t ReadUint32Binary(FILE* file);
    static void AppendInt32Binary(FILE* file, const int32_t& number);
    static void AppendUint32Binary(FILE* file, const uint32_t& number);
    static const int16_t ReadInt16Binary(FILE* file);
    static const uint16_t ReadUint16Binary(FILE* file);
    static void AppendInt16Binary(FILE* file, const int16_t& number);
    static void AppendUint16Binary(FILE* file, const uint16_t& number);
    static const int8_t ReadInt8Binary(FILE* file);
    static const uint8_t ReadUint8Binary(FILE* file);
    static void AppendInt8Binary(FILE* file, const int8_t& number);
    static void AppendUint8Binary(FILE* file, const uint8_t& number);
};

// START IMPLEMENTATION
// ==========================================================================================================

std::ifstream FileUtils::OpenRead(const char* filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return file;
}

std::wifstream FileUtils::OpenReadWide(const char* filepath)
{
    std::wifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return file;
}

std::ofstream FileUtils::OpenWrite(const char* filepath)
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return file;
}

std::wofstream FileUtils::OpenWriteWide(const char* filepath)
{
    std::wofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return file;
}

FILE* FileUtils::OpenReadBinary(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return f;
}

FILE* FileUtils::OpenWriteBinary(const char* filepath)
{
    FILE* f = fopen(filepath, "wb");
    if (f == NULL) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return f;
}

// ==========================================================================================================

void FileUtils::CloseFile(std::ifstream& file) {
    if (file.is_open()) {
        file.close();
    }
}

void FileUtils::CloseFile(std::ofstream& file) {
    if (file.is_open()) {
        file.close();
    }
}

void FileUtils::CloseFile(std::wifstream& file) {
    if (file.is_open()) {
        file.close();
    }
}

void FileUtils::CloseFile(std::wofstream& file) {
    if (file.is_open()) {
        file.close();
    }
}

void FileUtils::CloseFile(FILE* file) {
    if (file != NULL) {
        fclose(file);
    }
}

// ==========================================================================================================

// checks if file contains wide characters
bool FileUtils::ContainsWideChars(const char* filePath)
{
    std::wifstream file = OpenReadWide(filePath);

    std::wstring content = ReadWideContent(filePath);
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] >= 256) {
            return true;
        }
    }

    file.close();
    return false;
}

// ==========================================================================================

// read all wide characters from file to std::wstring
const std::wstring FileUtils::ReadWideContent(const char* filepath)
{
    std::wifstream file = OpenReadWide(filepath);

    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << file.rdbuf();
    file.close();

    return wss.str();
}

// read all characters from file to std::string
const std::string FileUtils::ReadContent(const char* filepath)
{
    std::ifstream file = OpenRead(filepath);
    std::string result = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return result;
}

// write std::string to file
void FileUtils::WriteContent(const char* filepath, const std::string& content)
{
    std::ofstream file = OpenWrite(filepath);
    file << content;
    file.close();
}

// write std::wstring to file

void FileUtils::WriteWideContent(const char* filepath, const std::wstring& content)
{
    std::wofstream file = OpenWriteWide(filepath);
    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    file << content;
    file.close();
}


// ==========================================================================================================

// append std::string to opened file
void FileUtils::AppendStr(std::ofstream& file, const std::string& str)
{
    file << str;
}

// append std::wstring to opened file
void FileUtils::AppendWideStr(std::wofstream& file, const std::wstring& str)
{
    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    file << str;
}

// ==========================================================================================================

// read character (wide character) from file in binary mode
const wchar_t FileUtils::ReadWideCharBinary(FILE* file)
{
    wchar_t c;
    fread(&c, sizeof(wchar_t), 1, file);
    return c;
}

// append character (wide character) to file in binary mode
void FileUtils::AppendWideCharBinary(FILE* file, const wchar_t& wc)
{
    fwrite(&wc, sizeof(wchar_t), 1, file);
}

// read character from file in binary mode
const char FileUtils::ReadCharBinary(FILE* file)
{
    char c;
    fread(&c, sizeof(char), 1, file);
    return c;
}

// append character to file in binary mode
void FileUtils::AppendCharBinary(FILE* file, const char& c)
{
    fwrite(&c, sizeof(char), 1, file);
}

// ==========================================================================================================

// read std::wstring of fixed size from file in binary mode
const std::wstring FileUtils::ReadWideStrBinary(FILE* file, const size_t& size)
{
    std::wstring str;
    str.reserve(size);
    // fread(&str, sizeof(wchar_t), size, f);
    for (size_t i = 0; i < size; ++i) {
        str.push_back(ReadWideCharBinary(file));
    }
    return str;
}

// write std::wstring to file in binary mode
void FileUtils::AppendWideStrBinary(FILE* file, const std::wstring& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        AppendWideCharBinary(file, str[i]);
    }
}

const std::string FileUtils::ReadStrBinary(FILE* file, const size_t& size)
{
    std::string str;
    str.reserve(size);
    // fread(&str, sizeof(char), size, f);
    for (size_t i = 0; i < size; ++i) {
        str.push_back(ReadCharBinary(file));
    }
    return str;
}

void FileUtils::AppendStrBinary(FILE* file, const std::string& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        AppendCharBinary(file, str[i]);
    }
}

// ==========================================================================================================

// read 64 bit int number from file in binary mode
const int64_t FileUtils::ReadInt64Binary(FILE* file)
{
    int64_t number;
    fread(&number, sizeof(int64_t), 1, file);
    return number;
}

// read 64 bit unsigned int number from file in binary mode
const uint64_t FileUtils::ReadUint64Binary(FILE* file)
{
    uint64_t number;
    fread(&number, sizeof(uint64_t), 1, file);
    return number;
}

// append 64 bit int number to file in binary mode
void FileUtils::AppendInt64Binary(FILE* file, const int64_t& number)
{
    fwrite(&number, sizeof(int64_t), 1, file);
}

// append 64 bit unsigned int number to file in binary mode
void FileUtils::AppendUint64Binary(FILE* file, const uint64_t& number)
{
    fwrite(&number, sizeof(uint64_t), 1, file);
}

// ==========================================================================================================

// read 32 bit int number from file in binary mode
const int32_t FileUtils::ReadInt32Binary(FILE* file)
{
    int32_t number;
    fread(&number, sizeof(int32_t), 1, file);
    return number;
}

// read 32 bit unsigned int number from file in binary mode
const uint32_t FileUtils::ReadUint32Binary(FILE* file)
{
    uint32_t number;
    fread(&number, sizeof(uint32_t), 1, file);
    return number;
}

// append 32 bit int number to file in binary mode
void FileUtils::AppendInt32Binary(FILE* file, const int32_t& number)
{
    fwrite(&number, sizeof(int32_t), 1, file);
}

// append 32 bit unsigned int number to file in binary mode
void FileUtils::AppendUint32Binary(FILE* file, const uint32_t& number)
{
    fwrite(&number, sizeof(uint32_t), 1, file);
}

// ==========================================================================================================

// read 16 bit int number from file in binary mode
const int16_t FileUtils::ReadInt16Binary(FILE* file)
{
    int16_t number;
    fread(&number, sizeof(int16_t), 1, file);
    return number;
}

// read 16 bit unsigned int number from file in binary mode
const uint16_t FileUtils::ReadUint16Binary(FILE* file)
{
    uint16_t number;
    fread(&number, sizeof(uint16_t), 1, file);
    return number;
}

// append 16 bit int number to file in binary mode
void FileUtils::AppendInt16Binary(FILE* file, const int16_t& number)
{
    fwrite(&number, sizeof(int16_t), 1, file);
}

// append 16 bit unsigned int number to file in binary mode
void FileUtils::AppendUint16Binary(FILE* file, const uint16_t& number)
{
    fwrite(&number, sizeof(uint16_t), 1, file);
}

// ==========================================================================================================

// read 8 bit int number from file in binary mode
const int8_t FileUtils::ReadInt8Binary(FILE* file)
{
    int8_t number;
    fread(&number, sizeof(int8_t), 1, file);
    return number;
}

// read 8 bit unsigned int number from file in binary mode
const uint8_t FileUtils::ReadUint8Binary(FILE* file)
{
    uint8_t number;
    fread(&number, sizeof(uint8_t), 1, file);
    return number;
}

// append 8 bit int number to file in binary mode
void FileUtils::AppendInt8Binary(FILE* file, const int8_t& number)
{
    fwrite(&number, sizeof(int8_t), 1, file);
}

// append 8 bit unsigned int number to file in binary mode
void FileUtils::AppendUint8Binary(FILE* file, const uint8_t& number)
{
    fwrite(&number, sizeof(uint8_t), 1, file);
}

// ==========================================================================================================
// END IMPLEMENTATION