#pragma once

/*
    This file is using for test encode functions
    (see all the encodings in .txt files)
*/

#include <string>
#include <fstream>
#include <set> // makes ordered set

#include "MyFile.h"
#include "TextTools.h"

// public functionality
std::wstring EncodeTxt(const std::string& key, const std::wstring& str);
std::wstring EncodeTxt(const std::string& key, const std::string& inputPath, const std::string& outputPath);

// private functionality
std::wstring EncodeRLE(const std::wstring& str); // Run-length encoding
std::wstring EncodeMTF(const std::wstring& str); // Move-to-front encoding
std::wstring EncodeBWT(const std::wstring& str); // Burrows-Wheeler transform
std::wstring EncodeAFM(const std::wstring& str); // Ariphmetical encoding

// START IMPLEMENTATION

std::wstring EncodeTxt(const std::string& key, const std::wstring& str)
{
    if (key == "RLE") {
        return EncodeRLE(str);
    } else if (key == "MTF") {
        return EncodeMTF(str);
    } else if (key == "BWT") {
        return EncodeBWT(str);
    } else if (key == "AFM") {
        return EncodeAFM(str);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return L"";
    }
}

std::wstring EncodeTxt(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    MyFile inputFile(inputPath, "r");
    std::wstring inputStr = inputFile.ReadWideContent();
    std::wstring outputStr = EncodeTxt(key, inputStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(outputStr);
    
    return outputStr;
}

std::wstring EncodeRLE(const std::wstring& str)
{
    std::wstring newStr = L"";

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::wstring uniqueSeq(1, str[0]); // last sequence of unique characters

    // show if previous character was part of sequence
    bool flag = false;

    wchar_t prev = str[0]; // previous character

    // start RLE
    for (size_t i = 1; i < str.size(); ++i)
    {
        if (str[i] == prev) 
        {
            // record last sequence of unique symbols if it exists
            if (countUnique > 1) {
                uniqueSeq.pop_back(); // because "prev" was read as unique
                --countUnique; // because "prev" was read as unique

                countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
                newStr += (std::to_wstring(-1 * countUnique) + uniqueSeq);

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { ++countIdent; }
            
            countUnique = 0;
            uniqueSeq = L"";
        }
        else 
        {
            // record last sequence of identical symbols if it exists
            if (countIdent > 1) {
                if (countIdent >= 9) {
                    for (size_t i = 0; i < (countIdent / 9); i++) {
                        newStr.push_back(L'9');
                        newStr.push_back(prev);
                    }
                }
                if (countIdent % 9 != 0) {
                    newStr.push_back((L'0' + (countIdent % 9)));
                    newStr.push_back(prev);
                }
                flag = true;
                countIdent = 1;
            } else if (countIdent == 0) {
                countIdent = 1;
            }

            if (flag) {
                countUnique = 1;
                uniqueSeq = str[i];
                flag = false;
            } else {
                if (countUnique == 0) {
                    countUnique = 1;
                    uniqueSeq = prev;
                }

                ++countUnique;
                uniqueSeq.push_back(str[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == 9) {
                newStr.append(std::to_wstring(-1 * countUnique) + uniqueSeq);
                flag = true;
                countUnique = 0;
                uniqueSeq = L"";
            }
        }
        prev = str[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= 9) {
            for (size_t i = 0; i < (countIdent / 9); ++i) {
                newStr.push_back(L'9');
                newStr.push_back(prev);
            }
        }
        if (countIdent % 9 != 0) {
            newStr.push_back((L'0' + (countIdent % 9)));
            newStr.push_back(prev);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        newStr.append(std::to_wstring(-1 * countUnique) + uniqueSeq);
    }

    return newStr;
}

std::wstring EncodeMTF(const std::wstring& str)
{
    std::wstring encodedStr;
    wchar_t* alphabet = Alphabet(str);
    int alphabetLength = wcslen(alphabet);

    // write length of alphabet
    encodedStr.append(std::to_wstring(alphabetLength) + L'\n');
    // write alphabet
    for (int i = 0; i < alphabetLength; ++i) {
        encodedStr.push_back(alphabet[i]);
    }
    encodedStr += '\n';

    // move-to-front
    for (size_t i = 0; i < str.size(); ++i) {
        int index = GetIndex(alphabet, alphabetLength, str[i]);
        encodedStr += std::to_wstring(index) + L' ';

        // shift to the right
        wchar_t temp = alphabet[0];
        for (size_t j = 1; j <= index; ++j) {
            wchar_t temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;
    }

    return encodedStr;
}

std::wstring EncodeBWT(const std::wstring& str)
{
    size_t permutationsLength = str.size();
    std::wstring* permutations = new std::wstring[permutationsLength];

    // generate permutations
    permutations[0] = str;
    for (size_t i = 1; i < permutationsLength; ++i) {
        permutations[i] = str.substr(i, permutationsLength - i) + str.substr(0, i);
    }
    
    // sort permutations
    std::sort(permutations, permutations + permutationsLength);

    // return result
    std::wstring result = L"";
    size_t indexOfOrignal;
    for (size_t i = 0; i < permutationsLength; ++i) {
        result += permutations[i][permutationsLength - 1];
        if (permutations[i] == str) indexOfOrignal = i;
    }
    result.insert(0, std::to_wstring(indexOfOrignal) + L"\n");

    delete[] permutations;
    return result;
}

std::wstring EncodeAFM(const std::wstring& str)
{
    auto encode = [](const std::wstring& str) {
        // initialize sorted alphabet and sorted frequencies
        wchar_t* alphabet = Alphabet(str);
        int size = wcslen(alphabet);
        std::pair<wchar_t, double>* frequencies = Frequencies(alphabet, size, str);
        std::sort(frequencies, frequencies + size);

        // leave in frequencies only 2 characters after the decimal point
        // (for correct decoding)
        for (int i = 0; i < size; ++i) {
            frequencies[i].second = ((int)(frequencies[i].second * 100)) / 100.0;
        }

        // inicialize segments
        //// (array of bounds points from 0 to 1)
        double* segments = new double[size + 1]{ 0 };
        for (int i = 1; i < size; ++i) {
            segments[i] = frequencies[i - 1].second + segments[i - 1];
        }
        segments[size] = 1;

        // encode
        double leftBound = 0, rightBound = 1, distance;
        for (wchar_t c : str) {
            int index = GetIndexInSorted(frequencies, size, c);
            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        // make result
        std::wstring result = std::to_wstring(size) + L'\n';
        for (int i = 0; i < size; ++i) {
            result.push_back(alphabet[i]);
        }
        result.push_back('\n'); 
        for (int i = 0; i < size; ++i){
            result += std::to_wstring((int8_t)((frequencies[i].second) * 100)) + L' ';
        }
        result.push_back('\n');

        double resultValue = (rightBound + leftBound) / 2;
        // leave only 9 digits after the decimal point
        //// cause int value can store any number with 9 digits 
        result += std::to_wstring((int)(resultValue * 1000000000));

        delete[] alphabet; delete[] frequencies; delete[] segments;
        return result;
    };

    std::wstring result = L"";
    size_t size = str.size();

    // write count of sequences
    size_t countOfSequences = (size % 9 == 0) ? (size / 9) : (size / 9 + 1);
    result += std::to_wstring(countOfSequences) + L'\n';

    // encode every 9 chars
    size_t i = 0;
    while (i + 9 <= size) {
        result += encode(str.substr(i, 9)) + L'\n';
        i += 9;
    }
    // handle the rest of the string
    if (i != size) {
        result += encode(str.substr(i, size - i)) + L'\n';
        result += std::to_wstring(size - i) + L'\n'; // fix last count of characters (cause it lower that 9)
    } else {
        result += std::to_wstring(9) + L'\n';
    }

    return result;
}

// END IMPLEMENTATION