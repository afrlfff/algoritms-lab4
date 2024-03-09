#pragma once

#include <string>

// mean length of sequences of repeating characters
double MeanRepeatingCharSeqLength(const std::wstring& str){
    size_t seqsCount = 0; // number of sequences
    size_t charsCount = 0; // number of characters in sequences
    size_t i = 0;
    while (i < str.size() - 1) {
        if (str[i] == str[i + 1]) {
            ++seqsCount;
            ++charsCount; // fix the fisrt char
            while (str[i] == str[i + 1]) {
                ++charsCount; ++i;
            }
        }
        ++i;
    }

    return (seqsCount == 0) ? 0 : (static_cast<double>(charsCount) / seqsCount);
}