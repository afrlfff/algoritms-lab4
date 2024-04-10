#pragma once

#include <string>
#include <set> // makes ordered set
#include <map>
#include <algorithm> // sorting
#include <cmath>

// Calculate entropy of the string
double GetTextEntropy(const std::u32string& str);

// return ordered alphabet from the string
std::u32string GetAlphabet(const std::u32string& str);
std::set<char32_t> GetAlphabetSet(const std::u32string& str);

// return map of characters and their frequencies
std::map<char32_t, double> GetCharFrequenciesMap(const std::u32string& alphabet, const size_t& size, const std::u32string& str);
std::map<char32_t, size_t> GetCharCountsMap(const std::u32string& alphabet, const size_t& size, const std::u32string& str);

// ratio of sequences of repeating characters within the string
double GetRepeatingCharSeqRatio(const std::u32string& str);

// mean length of sequences of repeating characters
double GetMeanRepeatingCharSeqLength(const std::u32string& str);


// START IMPLEMENTATION

double GetTextEntropy(const std::u32string& str) {
    std::map<char32_t, size_t> charCounts;
    size_t countOfChars = 0;
    char32_t c;

    // Calculate count of each character
    for (const char32_t& c : str) {
        countOfChars++;
        ++charCounts[c];
    }

    // Calculate probabilities and entropy
    double entropy = 0.0;
    for (const auto& pair : charCounts) {
        double probability = static_cast<double>(pair.second) / countOfChars;
        entropy -= probability * std::log2(probability);
    }

    return entropy;
}

std::u32string GetAlphabet(const std::u32string& str)
{
    const char32_t* charStr = str.c_str();
    std::set<char32_t> charsSet(charStr, charStr + str.size());

    std::u32string alphabet; alphabet.reserve(charsSet.size() + 1);

    for (char32_t c : charsSet) { 
        alphabet.push_back(c);
    }

    std::sort(alphabet.begin(), alphabet.end());
    return alphabet;
}

std::set<char32_t> GetAlphabetSet(const std::u32string& str)
{
    const char32_t* charStr = str.c_str();
    std::set<char32_t> alphabet(charStr, charStr + str.size());

    return alphabet;
}

std::map<char32_t, double> GetCharFrequenciesMap(const std::u32string& alphabet, const size_t& size, const std::u32string& str)
{
    std::map<char32_t, double> charFrequencies;

    size_t countAll = 0;
    for (char32_t c : str) {
        ++charFrequencies[c];
        ++countAll;
    } for (size_t i = 0; i < size; i++) {
        charFrequencies[alphabet[i]] /= static_cast<double>(countAll);
    }

    return charFrequencies;
}

std::map<char32_t, size_t> GetCharCountsMap(const std::u32string& alphabet, const size_t& size, const std::u32string& str)
{
    std::map<char32_t, size_t> charCounts;

    size_t countAll = 0;
    for (char32_t c : str) {
        ++charCounts[c];
        ++countAll;
    }

    return charCounts;
}

double GetRepeatingCharSeqRatio(const std::u32string& str){
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

    return (str.size() == 0) ? 0 : (static_cast<double>(charsCount - 2 * seqsCount) / str.size());
}

double GetMeanRepeatingCharSeqLength(const std::u32string& str){
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

// END IMPLEMENTATION