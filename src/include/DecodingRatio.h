#pragma once

#include <iostream>
#include <fstream>

double DecodingRatio(const std::string& pathToDecoded, const std::string& pathToOriginal)
{
    std::ifstream decodedFile(pathToDecoded, std::ios::binary);
    std::ifstream originalFile(pathToOriginal, std::ios::binary);
    if (!decodedFile.is_open() || !originalFile.is_open()){
        std::cout << "Error opening file in decodingRatio()" << std::endl;
        return 1;
    }

    decodedFile.seekg(0, std::ios::end);
    originalFile.seekg(0, std::ios::end);
    int decodedSize = decodedFile.tellg();
    int originalSize = originalFile.tellg();

    decodedFile.seekg(0, std::ios::beg);
    originalFile.seekg(0, std::ios::beg);
    int minSize = (decodedSize < originalSize) ? decodedSize : originalSize;
    int maxSize = (decodedSize > originalSize) ? decodedSize : originalSize;

    int count = 0;
    for (int i = 0; i < minSize; i++){
        if (decodedFile.get() == originalFile.get()){
            count++;
        }
    }

    return static_cast<double>(count) / maxSize;
}