#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <cmath>

double Entropy(std::string inputPath) {
    std::ifstream in(inputPath, std::ios::binary);
    if (!in.is_open()) {
        std::cout << "Error opening file: " << inputPath << std::endl;
        return 1;
    }

    std::map<char, int> charCounts;
    int countOfChars = 0;
    char c;

    // Calculate count of each character
    while (in.get(c)) {
        countOfChars++;
        charCounts[c]++;
    }
    in.close();

    // Calculate probabilities and entropy
    double entropy = 0.0;
    for (const auto& pair : charCounts) {
        double probability = static_cast<double>(pair.second) / countOfChars;
        entropy -= probability * std::log2(probability);
    }

    return entropy;
}