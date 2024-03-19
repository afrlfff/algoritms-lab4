#pragma once

#include <iostream>
#include <fstream>

#include "FilesProcessing.h"

double CompressionRatio(std::string originalPath, std::string compressedPath)
{
    std::filesystem::path origPath = originalPath;
    std::filesystem::path compPath = compressedPath;

    if (std::filesystem::exists(origPath) && std::filesystem::exists(compPath))
    {
        return static_cast<double>(std::filesystem::file_size(origPath)) / 
                std::filesystem::file_size(compPath);
    } else {
        std::cout << "CompressionRatio() Error: File doesn't exist" << std::endl;
        return -1.0;
    }
}