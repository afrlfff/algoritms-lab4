#pragma once

#include <iostream>
#include <fstream>

#include "ReadFileUtf8.h"

double CompressionRatio(std::string originalPath, std::string compressedPath)
{
    std::wstring originalContent = ReadFileUtf8(originalPath);
    std::wstring compressedContent = ReadFileUtf8(compressedPath);

    return static_cast<double>(originalContent.size()) / compressedContent.size();
}