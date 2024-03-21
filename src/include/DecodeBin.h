#pragma once

/*
    This file is using for test decode functions
*/

#include <string>
#include <algorithm>
#include <cstdint> // for int8_t

#include "FilesProcessing.h"
#include "TextTools.h"

// public functionality
std::wstring DecodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath);

// private functionality
std::wstring DecodeRLE(const std::string& inputPath); // Run-length encoding
std::wstring DecodeMTF(const std::string& inputPath); // Move-to-front
std::wstring DecodeBWT(const std::string& inputPath); // Burrows-Wheeler transform
std::wstring DecodeAFM(const std::string& inputPath); // Ariphmetical encoding


std::wstring DecodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    std::wstring result = L"";

    if (key == "RLE") {
        result = DecodeRLE(inputPath);
    } else if (key == "MTF") {
        result = DecodeMTF(inputPath);
    } else if (key == "BWT") {
        //return DecodeBWT(inputStr, outputPath);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return L"";
    }

    WriteWideContent(result, outputPath);

    return result;
}

std::wstring DecodeRLE(const std::string& inputPath)
{
    FILE* f = fopen(inputPath.c_str(), "rb");
    std::wstring newStr = L"";

    int8_t count;

    while (true)
    {
        count = ReadInt8Binary(f);

        // if count == 0 - end of file
        if (count == 0) break;

        // if starts with negative number
        // (sequence of unqiue symbols)
        if (count < 0)
        {
            for (int8_t i = 0; i < (-count); ++i) {
                newStr.push_back(ReadWideCharBinary(f));
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            wchar_t c = ReadWideCharBinary(f);
            for (int8_t i = 0; i < count; ++i) {
                newStr.push_back(c);
            }
        }
    }

    fclose(f);
    return newStr;
}

std::wstring DecodeMTF(const std::string& inputPath)
{
    auto DecodeMTF8 = [](wchar_t*& alphabet, const int64_t strLength, FILE*& f) {
        std::wstring decodedStr = L"";
        uint8_t index;
        wchar_t temp, temp2;
        for (int64_t i = 0; i < strLength; ++i) {
            index = ReadUint8Binary(f);
            decodedStr.push_back(alphabet[index]);

            // shift right
            temp = alphabet[0];
            for (size_t j = 1; j <= index; ++j) {
                temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
        return decodedStr;
    };
    auto DecodeMTF16 = [](wchar_t*& alphabet, const int64_t strLength, FILE*& f) {
        std::wstring decodedStr = L"";
        for (int64_t i = 0; i < strLength; ++i) {
            int16_t index = ReadUint16Binary(f);
            decodedStr.push_back(alphabet[index]);

            // shift right
            wchar_t temp = alphabet[0];
            for (size_t j = 1; j <= index; ++j) {
                wchar_t temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
        return decodedStr;
    };

    FILE* f = fopen(inputPath.c_str(), "rb");

    // read meta data
    uint16_t alphabetLength = ReadUint16Binary(f);
    wchar_t* alphabet = new wchar_t[alphabetLength + 1];
    for (size_t i = 0; i < alphabetLength; ++i) {
        alphabet[i] = ReadWideCharBinary(f);
    }
    alphabet[alphabetLength] = L'\0';
    int64_t strLength = ReadInt64Binary(f);

    // decode
    std::wstring decodedStr;
    if (alphabetLength <= 256) {
        decodedStr = DecodeMTF8(alphabet, strLength, f);
    } else {
        decodedStr = DecodeMTF16(alphabet, strLength, f);
    }

    fclose(f);
    return decodedStr;
}

// END