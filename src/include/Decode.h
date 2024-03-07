#pragma once

#include <string>

#include "ReadFileUtf8.h"
#include "WriteFileUtf8.h"

std::wstring DecodeRLE(std::wstring str);
std::wstring Decode(std::string key, std::wstring str);
std::wstring Decode(std::string key, std::string inputPath, std::string outputPath);
std::wstring DecodeMTF(const std::wstring& str);


std::wstring Decode(std::string key, std::wstring str)
{
    if (key == "RLE") {
        return DecodeRLE(str);
    } else if (key == "MTF") {
        return DecodeMTF(str);
    } else{
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

// Run-length decoding
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

// Move-to-front decoding
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


