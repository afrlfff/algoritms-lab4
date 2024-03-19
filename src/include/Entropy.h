#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <cmath>

#include "FilesProcessing.h"

// Calculate entropy of .txt file
double Entropy(std::string inputPath) {
    std::wstring content = ReadWideContent(inputPath);

    std::map<wchar_t, int> charCounts;
    int countOfChars = 0;
    wchar_t c;

    // Calculate count of each character
    for (const wchar_t& c : content) {
        countOfChars++;
        charCounts[c]++;
    }

    // Calculate probabilities and entropy
    double entropy = 0.0;
    for (const auto& pair : charCounts) {
        double probability = static_cast<double>(pair.second) / countOfChars;
        entropy -= probability * std::log2(probability);
    }

    return entropy;
}