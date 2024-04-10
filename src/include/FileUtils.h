#pragma once

#include <iostream>
#include <fstream>
#include <cmath>
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

    // non-binary files functions
    static const std::string ReadContentToString(const char* filepath);
    static const std::u32string ReadContentToU32String(const char* filepath);
    static void WriteContent(const char* filepath, const std::string& content);
    static void AppendStr(std::ofstream& file, const std::string& str);

    // binary files functions
    template <typename valueType>
    static const valueType ReadValueBinary(FILE* file);
    template <typename valueType>
    static void AppendValueBinary(FILE* file, const valueType number);
    static const std::string ReadStrBinary(FILE* file, const size_t& size);
    static void AppendStrBinary(FILE* file, const std::string& str);

    // complex functions
    static void AppendSequenceOfDigitsBinary(FILE* file, const std::string& str);
    static std::string ReadSequenceOfDigitsBinary(FILE* file, const size_t& size);
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

// ==========================================================================================================

// append std::string to opened file
void FileUtils::AppendStr(std::ofstream& file, const std::string& str)
{
    file << str;
}

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

const std::string FileUtils::ReadStrBinary(FILE* file, const size_t& size)
{
    std::string str; str.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        str.push_back(ReadValueBinary<char>(file));
    }

    return str;
}

void FileUtils::AppendStrBinary(FILE* file, const std::string& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        AppendValueBinary(file, str[i]);
    }
}

// ==========================================================================================================

void FileUtils::AppendSequenceOfDigitsBinary(FILE* file, const std::string& str)
{
    size_t stringPointer = 0;

    uint64_t ui64; // can store 19 digits
    uint32_t ui32; // can store 9 digits
    uint16_t ui16; // can store 4 digits
    uint8_t ui8; // can store 2 digits
    int8_t i8; // it will store 1 digit

    while (stringPointer < str.size()) {
        if ((str.size() - stringPointer) >= 19) {
            ui64 = 0;
            for (int i = 18; i >= 0; --i) {
                ui64 += static_cast<uint64_t>(std::pow(10, i)) * (str[stringPointer++] - '0');
            }
            AppendValueBinary(file, ui64);
        } else if ((str.size() - stringPointer) >= 9) {
            ui32 = 0;
            for (int i = 8; i >= 0; --i) {
                ui32 += std::pow(10, i) * (str[stringPointer++] - '0');
            }
            AppendValueBinary(file, ui32);
        } else if ((str.size() - stringPointer) >= 4) {
            ui16 = 0;
            for (int i = 3; i >= 0; --i) {
                ui16 += std::pow(10, i) * (str[stringPointer++] - '0');
            }
            AppendValueBinary(file, ui16);
        } else if ((str.size() - stringPointer) >= 2) {
            ui8 = 0;
            ui8 += 10 * (str[stringPointer++] - '0');
            ui8 += 1 * (str[stringPointer++] - '0');
            AppendValueBinary(file, ui8);
        } else {
            i8 = -1 * (str[stringPointer++] - '0');
            AppendValueBinary(file, i8);
        }
    }
}

std::string FileUtils::ReadSequenceOfDigitsBinary(FILE* file, const size_t& size)
{
    std::string str; str.reserve(size);
    
    size_t counter = size;

    uint64_t ui64; // can store 19 digits
    uint32_t ui32; // can store 9 digits
    uint16_t ui16; // can store 4 digits
    uint8_t ui8; // can store 2 digits
    int8_t i8; // it will store 1 digit

    while (counter > 0) {
        if (counter >= 19) {
            ui64 = ReadValueBinary<uint64_t>(file);
            for (int i = 18; i >= 0; --i) {
                int k = ui64 / static_cast<uint64_t>(std::pow(10, i));
                str.push_back('0' + k);
                ui64 -= k * static_cast<uint64_t>(std::pow(10, i));
            }
            counter -= 19;
        } else if (counter >= 9) {
            ui32 = ReadValueBinary<uint32_t>(file);
            for (int i = 8; i >= 0; --i) {
                int k = static_cast<int>(ui32 / std::pow(10, i));
                str.push_back('0' + k);
                ui32 -= k * std::pow(10, i);
            }
            counter -= 9;
        } else if (counter >= 4) {
            ui16 = ReadValueBinary<uint16_t>(file);
            for (int i = 3; i >= 0; --i) {
                int k = static_cast<int>(ui16 / std::pow(10, i));
                str.push_back('0' + k);
                ui16 -= k * std::pow(10, i);
            }
            counter -= 4;
        } else if (counter >= 2) {
            ui8 = ReadValueBinary<uint8_t>(file);
            str.push_back('0' + ui8 / 10);
            str.push_back('0' + ui8 % 10);
            counter -= 2;
        } else {
            i8 = ReadValueBinary<int8_t>(file);
            str.push_back('0' + (-1 * i8));
            counter -= 1;
        }
    }

    return str;
}

// ==========================================================================================================
// END IMPLEMENTATION