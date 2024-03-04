#pragma once

#include <iostream>
#include <fstream>

double CompressionRatio(std::string originalPath, std::string compressedPath)
{
    std::ifstream originalFile(originalPath, std::ios::binary);
    std::ifstream compressedFile(compressedPath, std::ios::binary);
    if (!originalFile.is_open() && !compressedFile.is_open()){
        std::cout << "Error opening file in compressionRatio()";
        return 1;
    }

    originalFile.seekg(0, std::ios::end);
    compressedFile.seekg(0, std::ios::end);
    int originalSize = originalFile.tellg();
    int compressedSize = compressedFile.tellg();

    originalFile.close();
    compressedFile.close();

    return static_cast<double>(originalSize) / compressedSize;
}