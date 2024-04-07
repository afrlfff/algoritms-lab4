#pragma once

#include <iostream>
#include <filesystem> // C++ 17 and more

#include "FileUtils.h"

double EncodingRatio(const std::u32string& originalStr, const std::u32string& encodedStr)
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
        throw std::runtime_error("CompressionRatio() Error: File doesn't exist");
        return -1.0;
    }
}

double DecodingRatio(const std::u32string& originalStr, const std::u32string& decodedStr)
{
    size_t minSize = (decodedStr.size() < originalStr.size()) ? decodedStr.size() : originalStr.size();
    size_t maxSize = (decodedStr.size() > originalStr.size()) ? decodedStr.size() : originalStr.size();

    size_t count = 0;
    for (size_t i = 0; i < minSize; ++i){
        if (decodedStr[i] == originalStr[i]){
            ++count;
        }
    }

    return static_cast<double>(count) / maxSize;    
}

double DecodingRatio(const std::filesystem::path& pathToOriginal, const std::filesystem::path& pathToDecoded)
{
    std::u32string originalContent = FileUtils::ReadContentToU32String(pathToOriginal.string().c_str());
    std::u32string decodedContent = FileUtils::ReadContentToU32String(pathToDecoded.string().c_str());

    return DecodingRatio(originalContent, decodedContent);
}
