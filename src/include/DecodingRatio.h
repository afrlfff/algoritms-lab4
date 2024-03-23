#pragma once

#include <iostream>
#include <fstream>

#include "MyFile.h"

double DecodingRatio(const std::string& pathToDecoded, const std::string& pathToOriginal)
{
    MyFile decodedFile(pathToDecoded, "r");
    MyFile originalFile(pathToOriginal, "r");
    std::wstring decodedContent = decodedFile.ReadWideContent();
    std::wstring originalContent = originalFile.ReadWideContent();

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