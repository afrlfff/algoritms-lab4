#pragma once

#include <iostream>
#include <filesystem> // C++ 17 and more

#include "MyFile.h"


double EncodingRatio(const std::wstring& originalStr, const std::wstring& encodedStr)
{
    return static_cast<double>(originalStr.size()) / encodedStr.size();
}

double EncodingRatio(const std::filesystem::path& originalPath, const std::filesystem::path& encodedPath)
{
    if (std::filesystem::exists(originalPath) && std::filesystem::exists(encodedPath))
    {
        return static_cast<double>(std::filesystem::file_size(originalPath)) / 
                std::filesystem::file_size(encodedPath);
    } else {
        std::cout << "CompressionRatio() Error: File doesn't exist" << std::endl;
        return -1.0;
    }
}

double DecodingRatio(const std::wstring& originalStr, const std::wstring& decodedStr)
{
    int minSize = (decodedStr.size() < originalStr.size()) ? decodedStr.size() : originalStr.size();
    int maxSize = (decodedStr.size() > originalStr.size()) ? decodedStr.size() : originalStr.size();

    int count = 0;
    for (size_t i = 0; i < minSize; ++i){
        if (decodedStr[i] == originalStr[i]){
            ++count;
        }
    }

    return static_cast<double>(count) / maxSize;    
}

double DecodingRatio(const std::filesystem::path& pathToOriginal, const std::filesystem::path& pathToDecoded)
{
    MyFile originalFile(pathToOriginal.string(), "r");
    MyFile decodedFile(pathToDecoded.string(), "r");
    std::wstring originalContent = originalFile.ReadWideContent();
    std::wstring decodedContent = decodedFile.ReadWideContent();

    return DecodingRatio(originalContent, decodedContent);
}
