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
    template <typename fileType>
    static fileType OpenFile(const char* filepath);
    static FILE* OpenFileBinaryRead(const char* filepath);
    static FILE* OpenFileBinaryWrite(const char* filepath);
    
    template <typename fileType>
    static void CloseFile(fileType& file);
    static void CloseFile(FILE* file);

    //static bool ContainsWideChars(const char* filepath);

    // non-binary files functions
    static const std::string ReadContentToString(const char* filepath);
    static const std::u32string ReadContentToU32String(const char* filepath);
    //static const std::wstring ReadWideContent(const char* filepath);
    static void WriteContent(const char* filepath, const std::string& content);
    //static void WriteWideContent(const char* filepath, const std::wstring& content);
    static void AppendStr(std::ofstream& file, const std::string& str);
    //static void AppendWideStr(std::wofstream& file, const std::wstring& str);

    // binary files functions
    template <typename valueType>
    static const valueType ReadValueBinary(FILE* file);
    template <typename valueType>
    static void AppendValueBinary(FILE* file, const valueType number);
    template <typename stringType, typename equalCharType>
    static const stringType ReadStrBinary(FILE* file, const size_t& size);
    template <typename stringType>
    static void AppendStrBinary(FILE* file, const stringType& str);
};

// START IMPLEMENTATION
// ==========================================================================================================

template <typename fileType>
fileType FileUtils::OpenFile(const char* filepath)
{
    fileType file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return file;
}

FILE* FileUtils::OpenFileBinaryRead(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return f;
}

FILE* FileUtils::OpenFileBinaryWrite(const char* filepath)
{
    FILE* f = fopen(filepath, "wb");
    if (f == NULL) {
        throw std::runtime_error("Failed to open file " + std::string(filepath));
    }
    return f;
}

// ==========================================================================================================

template <typename fileType>
void FileUtils::CloseFile(fileType& file)
{
    // if it's C++ type of file
    if (file.is_open()) {
        file.close();
    }
}

void FileUtils::CloseFile(FILE* file)
{
    // if it's C type of file
    if (file != NULL) {
        fclose(file);
    }
}

// ==========================================================================================================

/* // checks if file contains wide characters
bool FileUtils::ContainsWideChars(const char* filePath)
{
    std::wifstream file = OpenFile<std::wifstream>(filePath);

    std::wstring content = ReadWideContent(filePath);
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] >= 256) {
            return true;
        }
    }

    file.close();
    return false;
}
 */
// ==========================================================================================

/* // read all wide characters from file to std::wstring
const std::wstring FileUtils::ReadWideContent(const char* filepath)
{
    std::wifstream file = OpenFile<std::wifstream>(filepath);

    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << file.rdbuf();
    file.close();

    return wss.str();
}
 */
// read all characters from file to std::string
const std::string FileUtils::ReadContentToString(const char* filepath)
{
    std::ifstream file = OpenFile<std::ifstream>(filepath);
    std::string result = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    FileUtils::CloseFile(file);
    return result;
}

const std::u32string FileUtils::ReadContentToU32String(const char* filepath)
{
    std::ifstream file = FileUtils::OpenFile<std::ifstream>(filepath);
    
    std::stringstream buffer;
    buffer << file.rdbuf();

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string content = converter.from_bytes(buffer.str());
    FileUtils::CloseFile(file);

    return content;
}

// write std::string to file
void FileUtils::WriteContent(const char* filepath, const std::string& content)
{
    std::ofstream file = OpenFile<std::ofstream>(filepath);
    file << content;
    file.close();
}

// write std::wstring to file
/* 
void FileUtils::WriteWideContent(const char* filepath, const std::wstring& content)
{
    std::wofstream file = OpenFile<std::wofstream>(filepath);
    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    file << content;
    file.close();
}
 */

// ==========================================================================================================

// append std::string to opened file
void FileUtils::AppendStr(std::ofstream& file, const std::string& str)
{
    file << str;
}

/* // append std::wstring to opened file
void FileUtils::AppendWideStr(std::wofstream& file, const std::wstring& str)
{
    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    file << str;
}
 */
// ==========================================================================================================

template <typename valueType>
const valueType FileUtils::ReadValueBinary(FILE* file)
{
    valueType value;
    fread(&value, sizeof(valueType), 1, file);
    return value;
}

template <typename valueType>
void FileUtils::AppendValueBinary(FILE* file, const valueType value)
{
    fwrite(&value, sizeof(valueType), 1, file);
}

// ==========================================================================================================

template <typename stringType, typename equalCharType>
const stringType FileUtils::ReadStrBinary(FILE* file, const size_t& size)
{
    stringType str; str.reserve(size + 1);
    for (size_t i = 0; i < size; ++i) {
        str[i] = ReadValueBinary<equalCharType>(file);
    }
    str[size] = '\0';

    return str;
}

template <typename stringType>
void FileUtils::AppendStrBinary(FILE* file, const stringType& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        AppendValueBinary(file, str[i]);
    }
}

// ==========================================================================================================
// END IMPLEMENTATION