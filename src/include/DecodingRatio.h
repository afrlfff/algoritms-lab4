#pragma once

#include <iostream>
#include <fstream>

#include "FilesProcessing.h"

double DecodingRatio(const std::string& pathToDecoded, const std::string& pathToOriginal)
{
    std::wstring decodedContent = ReadWideContent(pathToDecoded);
    std::wstring originalContent = ReadWideContent(pathToOriginal);

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