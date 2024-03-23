#pragma once

/*
    This file contains final functions for encoding
    (convert from .txt to .bin files)
*/

#include <string>
#include <fstream>
#include <set> // makes ordered set
#include <algorithm> // sorting
#include <cstdint> // for int8_t

#include "MyFile.h"
#include "TextTools.h"

// public functionality
void EncodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath);

// private functionality
void EncodeRLE(const std::wstring& str, const std::string& outputPath); // Run-length encoding
void EncodeMTF(const std::wstring& str, const std::string& outputPath); // Move-to-front encoding
void EncodeBWT(const std::wstring& str, const std::string& outputPath); // Burrows-Wheeler transform


// START IMPLEMENTATION

void EncodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    MyFile file(inputPath, "r");
    std::wstring inputStr = file.ReadWideContent();

    if (key == "RLE") {
        EncodeRLE(inputStr, outputPath);
    } else if (key == "MTF") {
        return EncodeMTF(inputStr, outputPath);
    } else if (key == "BWT") {
        return EncodeBWT(inputStr, outputPath);
    } else if (key == "AFM") {
        //return EncodeAFM(inputStr, outputPath);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
    }
}

void EncodeRLE(const std::wstring& str, const std::string& outputPath)
{
    MyFile file(outputPath, "w");

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::wstring uniqueSeq(1, str[0]); // last sequence of unique characters
    bool flag = false; // show if previous character was part of sequence
    wchar_t prev = str[0]; // previous character

    int8_t maxPossibleNumber = 127; // maximum possible value of int8_t

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
                file.AppendInt8Binary(-countUnique);
                file.AppendWideStrBinary(uniqueSeq);

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
                if (countIdent >= maxPossibleNumber) {
                    for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                        file.AppendInt8Binary(maxPossibleNumber);
                        file.AppendWideCharBinary(prev);
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    file.AppendInt8Binary(countIdent % maxPossibleNumber);
                    file.AppendWideCharBinary(prev);
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

                countUnique++;
                uniqueSeq.push_back(str[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == maxPossibleNumber) {
                file.AppendInt8Binary(-countUnique);
                file.AppendWideStrBinary(uniqueSeq);
                flag = true;
                countUnique = 0;
                uniqueSeq = L"";
            }
        }
        prev = str[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= maxPossibleNumber) {
            for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                file.AppendInt8Binary(maxPossibleNumber);
                file.AppendWideCharBinary(prev);
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            file.AppendInt8Binary(countIdent % maxPossibleNumber);
            file.AppendWideCharBinary(prev);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        file.AppendInt8Binary(-countUnique);
        file.AppendWideStrBinary(uniqueSeq);
    }
}

void EncodeMTF(const std::wstring& str, const std::string& outputPath)
{
    auto EncodeMTF8 = [](wchar_t*& alphabet, size_t alphabetLength, const std::wstring& str, MyFile& f) {
        // write alphabet
        for (size_t i = 0; i < alphabetLength; ++i) {
            f.AppendWideCharBinary(alphabet[i]);
        }

        // write length of string
        f.AppendUint64Binary(str.size());
        // write move-to-front
        for (size_t i = 0; i < str.size(); ++i) {
            unsigned index = GetIndex(alphabet, alphabetLength, str[i]);
            f.AppendUint8Binary(index);

            // shift to the right
            wchar_t temp = alphabet[0];
            for (unsigned j = 1; j <= index; ++j) {
                wchar_t temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
    };
    auto EncodeMTF16 = [](wchar_t*& alphabet, size_t alphabetLength, const std::wstring& str, MyFile& f) {
        // write alphabet
        for (size_t i = 0; i < alphabetLength; ++i) {
            f.AppendWideCharBinary(alphabet[i]);
        }

        // write length of string
        f.AppendUint64Binary(str.size());
        // write move-to-front
        for (size_t i = 0; i < str.size(); ++i) {
            unsigned index = GetIndex(alphabet, alphabetLength, str[i]);
            f.AppendUint16Binary(index);

            // shift to the right
            wchar_t temp = alphabet[0];
            for (unsigned j = 1; j <= index; ++j) {
                wchar_t temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
    };

    MyFile file(outputPath, "w");

    wchar_t* alphabet = Alphabet(str);
    size_t alphabetLength = wcslen(alphabet);

    file.AppendUint16Binary(alphabetLength); // write length of alphabet
    if (alphabetLength <= 256) {
        EncodeMTF8(alphabet, alphabetLength, str, file);
    } else {
        EncodeMTF16(alphabet, alphabetLength, str, file);
    }
}

void EncodeBWT(const std::wstring& str, const std::string& outputPath)
{
    MyFile file(outputPath, "w");

    uint64_t permutationsLength = str.size();
    std::wstring* permutations = new std::wstring[permutationsLength];

    // generate permutations
    permutations[0] = str;
    for (uint64_t i = 1; i < permutationsLength; i++) {
        permutations[i] = str.substr(i, permutationsLength - i) + str.substr(0, i);
    }
    
    // sort permutations
    std::sort(permutations, permutations + permutationsLength);

    // write result
    file.AppendUint64Binary(permutationsLength); // write length of string
    int64_t indexOfOrignal;
    for (uint64_t i = 0; i < permutationsLength; ++i) {
        file.AppendWideCharBinary(permutations[i][permutationsLength - 1]);
        if (permutations[i] == str) indexOfOrignal = i;
    }
    file.AppendUint64Binary(indexOfOrignal);

    delete[] permutations;
}

// END IMPLEMENTATION