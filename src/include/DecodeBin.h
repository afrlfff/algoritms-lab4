#pragma once

/*
    This file is using for test decode functions
*/

#include <string>
#include <algorithm>
#include <cstdint> // for int8_t

#include "MyFile.h"
#include "TextTools.h"

// public functionality
std::wstring DecodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath);

// private functionality
std::wstring DecodeRLE(const std::string& inputPath); // Run-length encoding
std::wstring DecodeMTF(const std::string& inputPath); // Move-to-front
std::wstring DecodeBWT(const std::string& inputPath); // Burrows-Wheeler transform
std::wstring DecodeAFM(const std::string& inputPath); // Ariphmetical encoding


std::wstring DecodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    std::wstring result = L"";

    if (key == "RLE") {
        result = DecodeRLE(inputPath);
    } else if (key == "MTF") {
        result = DecodeMTF(inputPath);
    } else if (key == "BWT") {
        result = DecodeBWT(inputPath);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return L"";
    }

    MyFile file(outputPath, "w");
    file.WriteWideContent(result);

    return result;
}

std::wstring DecodeRLE(const std::string& inputPath)
{
    MyFile file(inputPath, "r");
    std::wstring newStr = L"";

    int8_t count;

    while (true)
    {
        count = file.ReadInt8Binary();

        // if count == 0 - end of file
        if (count == 0) break;

        // if starts with negative number
        // (sequence of unqiue symbols)
        if (count < 0)
        {
            for (int8_t i = 0; i < (-count); ++i) {
                newStr.push_back(file.ReadWideCharBinary());
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            wchar_t c = file.ReadWideCharBinary();
            for (int8_t i = 0; i < count; ++i) {
                newStr.push_back(c);
            }
        }
    }

    return newStr;
}

std::wstring DecodeMTF(const std::string& inputPath)
{
    auto DecodeMTF8 = [](wchar_t*& alphabet, const int64_t strLength, MyFile& f) {
        std::wstring decodedStr = L"";
        uint8_t index;
        wchar_t temp, temp2;
        for (int64_t i = 0; i < strLength; ++i) {
            index = f.ReadUint8Binary();
            decodedStr.push_back(alphabet[index]);

            // shift right
            temp = alphabet[0];
            for (size_t j = 1; j <= index; ++j) {
                temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
        return decodedStr;
    };
    auto DecodeMTF16 = [](wchar_t*& alphabet, const int64_t strLength, MyFile& f) {
        std::wstring decodedStr = L"";
        for (int64_t i = 0; i < strLength; ++i) {
            int16_t index = f.ReadUint16Binary();
            decodedStr.push_back(alphabet[index]);

            // shift right
            wchar_t temp = alphabet[0];
            for (size_t j = 1; j <= index; ++j) {
                wchar_t temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
        return decodedStr;
    };

    MyFile file(inputPath, "r");

    // read meta data
    uint16_t alphabetLength = file.ReadUint16Binary();
    wchar_t* alphabet = new wchar_t[alphabetLength + 1];
    for (size_t i = 0; i < alphabetLength; ++i) {
        alphabet[i] = file.ReadWideCharBinary();
    }
    alphabet[alphabetLength] = L'\0';
    int64_t strLength = file.ReadInt64Binary();

    // decode
    std::wstring decodedStr;
    if (alphabetLength <= 256) {
        decodedStr = DecodeMTF8(alphabet, strLength, file);
    } else {
        decodedStr = DecodeMTF16(alphabet, strLength, file);
    }

    return decodedStr;
}

std::wstring DecodeBWT(const std::string& inputPath)
{
    // START READ METADATA
    MyFile file(inputPath, "r");

    uint64_t permutationsLength = file.ReadUint64Binary();
    // get sorted letters
    wchar_t* sortedLetters = new wchar_t[permutationsLength];
    for (uint64_t i = 0; i < permutationsLength; ++i) {
        sortedLetters[i] = file.ReadWideCharBinary();
    }
    uint64_t indexOfOriginal = file.ReadUint64Binary();

    // END READ METADATA

    // get permutations
    std::wstring* permutations = new std::wstring[permutationsLength];
    for (uint64_t i = 0; i < permutationsLength; ++i) {
        // add new column
        for (uint64_t j = 0; j < permutationsLength; ++j) {
            permutations[j].insert(0, 1, sortedLetters[j]);
        }

        // sort permutations
        //// stable sort is necessary to save order
        std::stable_sort(permutations, permutations + permutationsLength);
    }

    // get count of the same letters before the letter[indexOfOriginal]
    uint64_t count = 0;
    for (int64_t i = 0; i < indexOfOriginal; ++i) {
        if (sortedLetters[i] == sortedLetters[indexOfOriginal]) ++count;
    }

    delete[] sortedLetters;
    return permutations[indexOfOriginal];
}

// END