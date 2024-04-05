#pragma once

#include <string>
#include <set> // makes ordered set
#include <map>
#include <algorithm> // sorting
#include <cmath>
#include <utility> // for std::pair

// Calculate entropy of the string
double TextEntropy(const std::wstring& str);

// return ordered alphabet from the string
wchar_t* Alphabet(const std::wstring& str);

// return frequencies of the characters in order of the alphabet
double* Frequencies(wchar_t* alphabet, const size_t size, const std::wstring& str);

// return frequencies of the characters in order of the alphabet
std::pair<wchar_t, double>* CharFrequencyPairs(wchar_t* alphabet, const size_t size, const std::wstring& str);

// return index of the character in the alphabet
unsigned GetIndex(const wchar_t* alphabet, const size_t size, wchar_t c);

// return index of the character in the frequencies
unsigned GetIndex(const std::pair<wchar_t, double>* frequencies, const size_t size, wchar_t c);

// return index of the character in sorted alphabet
unsigned GetIndexInSorted(const wchar_t* alphabet, const size_t size, wchar_t c);

// return index of the character in sorted frequencies
unsigned GetIndexInSorted(const std::pair<wchar_t, double>* frequencies, const size_t size, wchar_t c);

// ratio of sequences of repeating characters within the string
double RepeatingCharSeqRatio(const std::wstring& str);

// mean length of sequences of repeating characters
double MeanRepeatingCharSeqLength(const std::wstring& str);


// START IMPLEMENTATION

double TextEntropy(const std::wstring& str) {
    std::map<wchar_t, int> charCounts;
    int countOfChars = 0;
    wchar_t c;

    // Calculate count of each character
    for (const wchar_t& c : str) {
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

wchar_t* Alphabet(const std::wstring& str)
{
    const wchar_t* charStr = str.c_str();
    std::set<wchar_t> charsSet(charStr, charStr + wcslen(charStr));

    wchar_t* alphabet = new wchar_t[charsSet.size() + 1];
    int ind = 0;
    for (wchar_t c : charsSet) { 
        alphabet[ind++] = c;
    }
    alphabet[ind] = '\0'; // end of string

    std::sort(alphabet, alphabet + ind);

    return alphabet;
}

double* Frequencies(wchar_t* alphabet, const size_t size, const std::wstring& str)
{
    double* frequencies = new double[size];
    for (size_t i = 0; i < size; ++i) {
        frequencies[i] = 0;
    }

    size_t countAll = 0;
    for (wchar_t c : str) {
        ++frequencies[GetIndex(alphabet, size, c)];
        ++countAll;
    } for (size_t i = 0; i < size; ++i) {
        frequencies[i] /= countAll;
    }

    return frequencies;
}

std::pair<wchar_t, double>* CharFrequencyPairs(wchar_t* alphabet, const size_t size, const std::wstring& str)
{
    std::pair<wchar_t, double>* frequencies = new std::pair<wchar_t, double>[size];
    for (int i = 0; i < size; i++) {
        frequencies[i] = std::make_pair(alphabet[i], 0);
    }

    size_t countAll = 0;
    for (wchar_t c : str) {
        ++frequencies[GetIndex(alphabet, size, c)].second;
        ++countAll;
    } for (int i = 0; i < size; i++) {
        frequencies[i].second /= countAll;
    }

    return frequencies;
}

unsigned GetIndex(const wchar_t* alphabet, const size_t size, wchar_t c)
{
    for (int i = 0; i < size; ++i) {
        if (alphabet[i] == c) {
            return i;
        }
    }
    std::cout << "GetIndex() Error: character " << c << " was not found in the alphabet" << std::endl;
    return 0;
}

unsigned GetIndex(const std::pair<wchar_t, double>* charFrequenciesPair, const size_t size, wchar_t c)
{
    for (int i = 0; i < size; ++i) {
        if (charFrequenciesPair[i].first == c) {
            return i;
        }
    }
    return -1;
}

unsigned GetIndexInSorted(const wchar_t* alphabet, const size_t size, wchar_t c)
{
    // binary search
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (alphabet[mid] == c) {
            return mid;
        }
        if (alphabet[mid] < c) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

unsigned GetIndexInSorted(const std::pair<wchar_t, double>* charFrequenciesPair, const size_t size, wchar_t c)
{
    // binary search
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (charFrequenciesPair[mid].first == c) {
            return mid;
        }
        if (charFrequenciesPair[mid].first < c) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;
}

double RepeatingCharSeqRatio(const std::wstring& str){
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

// END IMPLEMENTATION