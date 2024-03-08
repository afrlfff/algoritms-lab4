#pragma once

#include <string>
#include <algorithm>

#include "ReadFileUtf8.h"
#include "WriteFileUtf8.h"

std::wstring Decode(std::string key, std::wstring str);
std::wstring Decode(std::string key, std::string inputPath, std::string outputPath);
std::wstring DecodeRLE(std::wstring str); // Run-length encoding
std::wstring DecodeMTF(const std::wstring& str); // Move-to-front
std::wstring DecodeBWT(const std::wstring& str); // Burrows-Wheeler transform


std::wstring Decode(std::string key, std::wstring str)
{
    if (key == "RLE") {
        return DecodeRLE(str);
    } else if (key == "MTF") {
        return DecodeMTF(str);
    } else if (key == "BWT") {
        return DecodeBWT(str);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return L"";
    }
}

std::wstring Decode(std::string key, std::string inputPath, std::string outputPath)
{
    std::wstring inputStr = ReadFileUtf8(inputPath);
    std::wstring outputStr = Decode(key, inputStr);
    WriteFileUtf8(outputStr, outputPath);

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
    // get count of '\n' in str effectively
    int newLineCount = 0, spaceCount = 0;
    for (wchar_t c : str) {
        if (c == L'\n') newLineCount++;
        else if (c == L' ') {
            spaceCount++;
            if (spaceCount == 2) break;
        }
    }

    // get alphabet length
    size_t alphabetLength = 0;
    int newLineCountTemp = 0;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == L'\n') {
            newLineCountTemp++;
            if (newLineCountTemp == newLineCount) break;
        }
        alphabetLength++;
    }

    // read alphabet
    wchar_t* alphabet = new wchar_t[alphabetLength + 1];
    size_t i = 0; newLineCountTemp = 0;
    while (i < str.size()) {
        if (str[i] == L'\n') {
            newLineCountTemp++;
            if (newLineCountTemp == newLineCount) {i++; break;}
        }
        alphabet[i] = str[i];
        i++;
    }
    alphabet[alphabetLength] = L'\0';

    // decode
    std::wstring decodedStr = L"";
    while (i < str.size()) {
        std::wstring numberStr = L"";
        while (str[i] != L' ') {
            numberStr.push_back(str[i++]);
        }
        size_t index = std::stoi(numberStr);
        decodedStr.push_back(alphabet[index]);

        // shift
        wchar_t temp = alphabet[0];
        for (size_t j = 1; j <= index; j++) {
            wchar_t temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;

        i++;
    }

    return decodedStr;
}

std::wstring DecodeBWT(const std::wstring& str)
{
    size_t index = -1, permutationsLength = 0;

    // get index and length of permutations
    for (size_t i = 0; i < str.size(); i++) {
        if (index != -1) permutationsLength++;
        if (str[i] == L'\n') {
            index  = std::stoi(str.substr(0, i));
        }
    }

    // get permutations
    std::wstring letters = str.substr(str.size() - permutationsLength, permutationsLength);
    std::wstring* permutations = new std::wstring[permutationsLength];
    for (size_t i = 0; i < permutationsLength; i++) {
        // add new column
        for (size_t j = 0; j < permutationsLength; j++) {
            permutations[j].insert(0, 1, letters[j]);
        }

        // sort permutations
        std::stable_sort(permutations, permutations + permutationsLength);
    }

    // get count of the same letters before the letter[index]
    size_t count = 0;
    for (size_t i = 0; i < index; i++) {
        if (letters[i] == letters[index]) count++;
    }

    // find the index of original string
    size_t newIndex = 0;
    size_t countTemp = 0;
    for (size_t i = 0; i < permutationsLength; i++) {
        if (permutations[i][permutationsLength -1] == letters[index]) {
            countTemp++;
            if (countTemp == count + 1) {
                newIndex = i;
                break;
            }
        }
    }

    return permutations[newIndex];
}



