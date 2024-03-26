#pragma once

/*
    This file is using for test decode functions
*/

#include <string>
#include <algorithm>

#include "MyFile.h"

// public functionality
std::wstring DecodeTxt(const std::string& key, const std::wstring& str);
std::wstring DecodeTxt(const std::string& key, const std::string& inputPath, const std::string& outputPath);

// private functionality
std::wstring DecodeRLE(std::wstring str); // Run-length encoding
std::wstring DecodeMTF(const std::wstring& str); // Move-to-front
std::wstring DecodeBWT(const std::wstring& str); // Burrows-Wheeler transform
std::wstring DecodeAFM(const std::wstring& str); // Ariphmetical encoding

std::wstring DecodeTxt(const std::string& key, const std::wstring& str)
{
    if (key == "RLE") {
        return DecodeRLE(str);
    } else if (key == "MTF") {
        return DecodeMTF(str);
    } else if (key == "BWT") {
        return DecodeBWT(str);
    } else if (key == "AFM") {
        return DecodeAFM(str);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return L"";
    }
}

std::wstring DecodeTxt(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    MyFile inputFile(inputPath, "r");
    std::wstring inputStr = inputFile.ReadWideContent();
    std::wstring outputStr = DecodeTxt(key, inputStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(outputStr);

    return outputStr;
}

std::wstring DecodeRLE(std::wstring str)
{
    std::wstring newStr = L"";

    size_t i = 0;
    int8_t count;

    while (i < str.length())
    {
        // if starts with negative number
        // (sequence of unqiue symbols)
        if (str[i] == '-')
        {
            count = str[i + 1] - L'0'; // convert char to int
            i += 2;
            for (size_t j = i; j < (i + count); j++) {
                newStr.push_back(str[j]);
            }
            i += count;
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            count = str[i] - L'0';
            newStr.append(count, str[i + 1]);
            i += 2;
        }
    }

    return newStr;
}

std::wstring DecodeMTF(const std::wstring& str)
{
    wchar_t c;
    size_t i = 0;
    int alphabetLength;
    wchar_t* alphabet;

    // read alphabet length
    wchar_t buffer[10];
    while (str[i] != L'\n') {
        buffer[i++] = str[i];
    }
    alphabetLength = std::stoi(buffer);

    // read alphabet
    ++i;
    alphabet = new wchar_t[alphabetLength + 1];
    for (size_t j = 0; j < alphabetLength; ++j) {
        alphabet[j] = str[i++];
    }
    alphabet[alphabetLength] = L'\0';

    // decode
    ++i;
    std::wstring decodedStr = L"";
    while (i < str.size()) {
        std::wstring numberStr = L"";
        while (str[i] != L' ') {
            numberStr.push_back(str[i++]);
        }
        size_t index = std::stoi(numberStr);
        decodedStr.push_back(alphabet[index]);

        // shift right
        wchar_t temp = alphabet[0];
        for (size_t j = 1; j <= index; ++j) {
            wchar_t temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;

        ++i;
    }

    delete[] alphabet;
    return decodedStr;
}

std::wstring DecodeBWT(const std::wstring& str)
{
    size_t index = -1, permutationsLength = 0;

    // get index and length of permutations
    for (size_t i = 0; i < str.size(); ++i) {
        if (index != -1) ++permutationsLength;
        if (str[i] == L'\n') {
            index  = std::stoi(str.substr(0, i));
        }
    }

    // get permutations
    std::wstring sortedLetters = str.substr(str.size() - permutationsLength, permutationsLength);
    std::wstring* permutations = new std::wstring[permutationsLength];
    for (size_t i = 0; i < permutationsLength; ++i) {
        // add new column
        for (size_t j = 0; j < permutationsLength; ++j) {
            permutations[j].insert(0, 1, sortedLetters[j]);
        }

        // sort permutations
        std::stable_sort(permutations, permutations + permutationsLength);
    }

    // get count of the same letters before the letter[index]
    size_t count = 0;
    for (size_t i = 0; i < index; i++) {
        if (sortedLetters[i] == sortedLetters[index]) count++;
    }

    return permutations[index];
}

std::wstring DecodeAFM(const std::wstring& str)
{
    auto decode = [](const wchar_t alhpabet[10], int alphabetSize, const double frequencies[9], double resultValue, int countOfIterations) {
        // inicialize segments
        //// (array of bound points from 0 to 1)
        double* segments = new double[alphabetSize + 1]{ 0 };
        for (int i = 1; i < alphabetSize; ++i) {
            segments[i] = frequencies[i - 1] + segments[i - 1];
        }
        segments[alphabetSize] = 1;

        // decode
        std::wstring result;
        double leftBound = 0, rightBound = 1, distance;
        int index;
        for (int i = 0; i < countOfIterations; ++i) {
            // find index of segment that contains resultValue
            for (int j = 0; j < alphabetSize; ++j) {
                if (resultValue >= (leftBound + segments[j] * (rightBound - leftBound)) && 
                    resultValue < (leftBound + segments[j + 1] * (rightBound - leftBound))) {
                    index = j;
                    break;
                }
            }
            result.push_back(alhpabet[index]);

            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        delete[] segments;
        return result;
    };

    size_t i = 0, // index in str
           bufferIndex; // index in buffer
    wchar_t buffer[20];
    std::wstring result = L""; 

    // get count of sequences
    size_t countOfSequences;
    bufferIndex = 0;
    while (str[i] != L'\n') {
        buffer[bufferIndex++] = str[i];
        ++i;
    }
    buffer[bufferIndex] = L'\0';
    ++i; // to skip '\n'
    countOfSequences = std::stoi(buffer);

    // get sequences and merge results
    int alphabetSize;
    wchar_t alhpabet[9 + 1]; // can't contain more that 9 characters
    double frequencies[9];
    double resultValue;
    int lastCount = 9;
    for (size_t j = 0; j < countOfSequences; ++j) {
        // read size of alphabet
        bufferIndex = 0;
        while (str[i] != L'\n') {
            buffer[bufferIndex++] = str[i];
            ++i;
        }
        buffer[bufferIndex] = L'\0';
        ++i; // i++ to skip '\n'
        alphabetSize = std::stoi(buffer);

        // read alhpabet
        for (int k = 0; k < alphabetSize; ++k) {
            alhpabet[k] = str[i];
            ++i;
        }
        alhpabet[alphabetSize] = L'\0';
        ++i; // to skip '\n'

        // read frequencies
        for (int k = 0; k < alphabetSize; ++k) {
            bufferIndex = 0;
            while (str[i] != L' ' && str[i] != L'\n') {
                buffer[bufferIndex++] = str[i];
                ++i;
            }

            buffer[bufferIndex++] = L'\0';
            frequencies[k] = std::stoi(buffer) / 100.0;
            ++i; // to skip ' '
        }

        // read result value
        ++i; // to skip '\n'
        bufferIndex = 0;
        while (str[i] != L'\n') {
            buffer[bufferIndex++] = str[i];
            ++i;
        }
        buffer[bufferIndex] = L'\0';
        ++i; // to skip '\n'
        resultValue = std::stoi(buffer) / 1000000000.0;

        // read last count if it exists
        if (j == countOfSequences - 1) { // if last iteration
            bufferIndex = 0;
            while (str[i] != L'\n') {
                buffer[bufferIndex++] = str[i];
                ++i;
            }
            buffer[bufferIndex] = L'\0';
            lastCount = std::stoi(buffer);
            ++i; // to skip '\n'
        }

        result += decode(alhpabet, alphabetSize, frequencies, resultValue, lastCount);
    }

    return result;
}


// END