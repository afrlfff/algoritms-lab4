#pragma once

/*
    This file is using for test decode functions
*/

#include <string>
#include <algorithm>
#include <cstdint> // for int8_t

#include "FilesProcessing.h"

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
        //result = DecodeMTF(inputPath);
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
        if (count == 0) break;
        //std::cout << count << std::endl;

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

    return newStr;
}

// END