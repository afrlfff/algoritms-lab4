#pragma once

#include <iostream>
#include <fstream>

#include "ReadFileUtf8.h"

double DecodingRatio(const std::string& pathToDecoded, const std::string& pathToOriginal)
{
    std::wstring decodedContent = ReadFileUtf8(pathToDecoded);
    std::wstring originalContent = ReadFileUtf8(pathToOriginal);

    int minSize = (decodedContent.size() < originalContent.size()) ? decodedContent.size() : originalContent.size();
    int maxSize = (decodedContent.size() > originalContent.size()) ? decodedContent.size() : originalContent.size();

    int count = 0;
    for (int i = 0; i < minSize; i++){
        if (decodedContent[i] == originalContent[i]){
            count++;
        }
    }

    return static_cast<double>(count) / maxSize;
}