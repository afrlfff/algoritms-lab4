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

#include "FilesProcessing.h"

// public functionality
void EncodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath);

// private functionality
void EncodeRLE(const std::wstring& str, const std::string& outputPath); // Run-length encoding


// START IMPLEMENTATION

void EncodeBin(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    std::wstring inputStr = ReadWideContent(inputPath);

    if (key == "RLE") {
        EncodeRLE(inputStr, outputPath);
    } else if (key == "MTF") {
        //return EncodeMTF(inputStr, outputPath);
    } else if (key == "BWT") {
        //return EncodeBWT(inputStr, outputPath);
    } else if (key == "AFM") {
        //return EncodeAFM(inputStr, outputPath);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
    }
}

void EncodeRLE(const std::wstring& str, const std::string& outputPath)
{
    FILE* f = fopen(outputPath.c_str(), "wb");

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
                AppendInt8Binary(-countUnique, f);
                AppendWideStrBinary(uniqueSeq, f);

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
                        AppendInt8Binary(maxPossibleNumber, f);
                        AppendWideCharBinary(prev, f);
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    AppendInt8Binary(countIdent % maxPossibleNumber, f);
                    AppendWideCharBinary(prev, f);
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
                AppendInt8Binary(-countUnique, f);
                AppendWideStrBinary(uniqueSeq, f);
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
                AppendInt8Binary(maxPossibleNumber, f);
                AppendWideCharBinary(prev, f);
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            AppendInt8Binary(countIdent % maxPossibleNumber, f);
            AppendWideCharBinary(prev, f);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        AppendInt8Binary(-countUnique, f);
        AppendWideStrBinary(uniqueSeq, f);
    }

    fclose(f);
}

// END IMPLEMENTATION