#pragma once

#include <iostream>
#include <fstream>

#include "FilesProcessing.h"

double CompressionRatio(std::string originalPath, std::string compressedPath)
{
    std::wstring originalContent = ReadWideContent(originalPath);
    std::wstring compressedContent = ReadWideContent(compressedPath);

    return static_cast<double>(originalContent.size()) / compressedContent.size();
}