#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <locale>

/**
 * Class for working with files containing wide charaters in C++
 * (was wrote cause there are many difficulties with wide characters processing in C++)
 * It wraps standart C file handling functions and C++ a little
 */
class MyFile
{
    char* filePath;
    FILE* fbin;
public:
    MyFile(const char* filePath, const char* mode);
    MyFile(const std::string& filePath, const char* mode);

    const std::wstring ReadWideContent();
    void WriteWideContent(const std::wstring& str);
    const std::wstring ReadWideStrBinary(const size_t& size);
    void AppendWideStrBinary(const std::wstring& str);
    const wchar_t ReadWideCharBinary();
    void AppendWideCharBinary(const wchar_t& c);
    const int64_t ReadInt64Binary();
    const uint64_t ReadUint64Binary();
    void AppendInt64Binary(const int64_t& number);
    void AppendUint64Binary(const uint64_t& number);
    const int32_t ReadInt32Binary();
    const uint32_t ReadUint32Binary();
    void AppendInt32Binary(const int32_t& number);
    void AppendUint32Binary(const uint32_t& number);
    const int16_t ReadInt16Binary();
    const uint16_t ReadUint16Binary();
    void AppendInt16Binary(const int16_t& number);
    void AppendUint16Binary(const uint16_t& number);
    const int8_t ReadInt8Binary();
    const uint8_t ReadUint8Binary();
    void AppendInt8Binary(const int8_t& number);
    void AppendUint8Binary(const uint8_t& number);
    
    ~MyFile() { fclose(fbin); delete[] filePath; };
};

// START IMPLEMENTATION OF THE CLASS MyFile
// ==========================================================================================================

/**
 * Constructor for the MyFile class.
 *
 * @param filePath The path to the file.
 * @param mode The mode in which to open the file ("r" or "w").
 */
MyFile::MyFile(const char* filePath, const char* mode)
{
    if (mode != "r" && mode != "w") {
        std::cout << "Error: Wrong mode" << std::endl;
        exit(1);
    }

    if (mode == "r")
        fbin = fopen(filePath, "rb");
    else
        fbin = fopen(filePath, "wb");

    int i = 0;
    while(filePath[i] != '\0') {
        this->filePath[i++] = filePath[i];
    }
    this->filePath[i] = '\0';

}

/**
 * Constructor for the MyFile class.
 *
 * @param filePath The path to the file.
 * @param mode The mode in which to open the file ("r" or "w").
 */
MyFile::MyFile(const std::string& filePath, const char* mode) { MyFile(filePath.c_str(), mode); }

// ==========================================================================================================

// read all characters (wide characters) from file to std::wstring
const std::wstring MyFile::ReadWideContent()
{
    std::wifstream wif(filePath);
    if (!wif.is_open()) {
        std::cout << "Error: File " << filePath << " doesn't exist" << std::endl;
        return L"";
    }

    wif.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
    std::wstringstream wss;
    wss << wif.rdbuf();
    wif.close();

    return wss.str();

    // I don't use the code below because it works incorrect with wchars
    // (i have no idea why)

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
void MyFile::WriteWideContent(const std::wstring& str)
{
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        std::cout << "Error: File " << filePath << " doesn't exist" << std::endl;
        return;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string utf8Str = converter.to_bytes(str);
    outFile.write(utf8Str.c_str(), utf8Str.size());

    // I don't use the code below because it works incorrect with wchars
    // (I have no idea why)

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

// read std::wstring of fixed size from file in binary mode
const std::wstring MyFile::ReadWideStrBinary(const size_t& size)
{
    fbin = fopen(filePath, "rb");

    std::wstring str;
    fread(&str, sizeof(wchar_t), size, fbin);
    return str;
}

// write std::wstring to file in binary mode
void MyFile::AppendWideStrBinary(const std::wstring& str)
{
    for (int i = 0; i < str.size(); ++i) {
        AppendWideCharBinary(str[i]);
    }
}

// ==========================================================================================================

// read character (wide character) from file in binary mode
const wchar_t MyFile::ReadWideCharBinary()
{
    wchar_t c;
    fread(&c, sizeof(wchar_t), 1, fbin);
    return c;
}

// append character (wide character) to file in binary mode
void MyFile::AppendWideCharBinary(const wchar_t& c)
{
    fwrite(&c, sizeof(wchar_t), 1, fbin);
}

// ==========================================================================================================

// read 64 bit int number from file in binary mode
const int64_t MyFile::ReadInt64Binary()
{
    int64_t number;
    fread(&number, sizeof(int64_t), 1, fbin);
    return number;
}

// read 64 bit unsigned int number from file in binary mode
const uint64_t MyFile::ReadUint64Binary()
{
    uint64_t number;
    fread(&number, sizeof(uint64_t), 1, fbin);
    return number;
}

// append 64 bit int number to file in binary mode
void MyFile::AppendInt64Binary(const int64_t& number)
{
    fwrite(&number, sizeof(int64_t), 1, fbin);
}

// append 64 bit unsigned int number to file in binary mode
void MyFile::AppendUint64Binary(const uint64_t& number)
{
    fwrite(&number, sizeof(uint64_t), 1, fbin);
}

// ==========================================================================================================

// read 32 bit int number from file in binary mode
const int32_t MyFile::ReadInt32Binary()
{
    int32_t number;
    fread(&number, sizeof(int32_t), 1, fbin);
    return number;
}

// read 32 bit unsigned int number from file in binary mode
const uint32_t MyFile::ReadUint32Binary()
{
    uint32_t number;
    fread(&number, sizeof(uint32_t), 1, fbin);
    return number;
}

// append 32 bit int number to file in binary mode
void MyFile::AppendInt32Binary(const int32_t& number)
{
    fwrite(&number, sizeof(int32_t), 1, fbin);
}

// append 32 bit unsigned int number to file in binary mode
void MyFile::AppendUint32Binary(const uint32_t& number)
{
    fwrite(&number, sizeof(uint32_t), 1, fbin);
}

// ==========================================================================================================

// read 16 bit int number from file in binary mode
const int16_t MyFile::ReadInt16Binary()
{
    int16_t number;
    fread(&number, sizeof(int16_t), 1, fbin);
    return number;
}

// read 16 bit unsigned int number from file in binary mode
const uint16_t MyFile::ReadUint16Binary()
{
    uint16_t number;
    fread(&number, sizeof(uint16_t), 1, fbin);
    return number;
}

// append 16 bit int number to file in binary mode
void MyFile::AppendInt16Binary(const int16_t& number)
{
    fwrite(&number, sizeof(int16_t), 1, fbin);
}

// append 16 bit unsigned int number to file in binary mode
void MyFile::AppendUint16Binary(const uint16_t& number)
{
    fwrite(&number, sizeof(uint16_t), 1, fbin);
}

// ==========================================================================================================

// read 8 bit int number from file in binary mode
const int8_t MyFile::ReadInt8Binary()
{
    int8_t number;
    fread(&number, sizeof(int8_t), 1, fbin);
    return number;
}

// read 8 bit unsigned int number from file in binary mode
const uint8_t MyFile::ReadUint8Binary()
{
    uint8_t number;
    fread(&number, sizeof(uint8_t), 1, fbin);
    return number;
}

// append 8 bit int number to file in binary mode
void MyFile::AppendInt8Binary(const int8_t& number)
{
    fwrite(&number, sizeof(int8_t), 1, fbin);
}

// append 8 bit unsigned int number to file in binary mode
void MyFile::AppendUint8Binary(const uint8_t& number)
{
    fwrite(&number, sizeof(uint8_t), 1, fbin);
}

// ==========================================================================================================
// END IMPLEMENTATION OF THE CLASS MyFile