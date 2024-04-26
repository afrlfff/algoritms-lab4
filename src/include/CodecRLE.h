#pragma once

#include <string>
#include <cstdint>
#include <queue>

#include "FileUtils.h"
#include "CodecUTF8.h"

// Run-length encoding
class CodecRLE
{
private:
    CodecRLE() = default;
public:
    static void Encode(const char* inputPath, const char* outputPath);
    static void Decode(const char* inputPath, const char* outputPath);

    template <typename valueType>
    struct data_numerical {
        uint64_t numLength;
        std::queue<std::pair<int8_t, std::vector<valueType>>> encodedNums;
        data_numerical(uint64_t _numLength, std::queue<std::pair<int8_t, std::vector<valueType>>> _encodedNums) : numLength(_numLength), encodedNums(_encodedNums) {}
    };

    // functions to work with MTF codec
    template <typename valueType>
    static data_numerical<valueType> GetDataNumerical(const std::vector<valueType>& inputNums);
    template <typename valueType>
    static std::vector<valueType> DecodeRLENumerical(FILE* inputFile);
protected:
    struct data {
        uint64_t strLength;
        std::queue<std::pair<int8_t, std::u32string>> encodedStr;
        data(uint64_t _strLength, std::queue<std::pair<int8_t, std::u32string>> _encodedStr) : strLength(_strLength), encodedStr(_encodedStr) {}
    };
    

    static data GetData(const std::u32string& inputStr);
    static std::string DecodeRLE(FILE* inputFile);

    
};



// START IMPLEMENTATION

CodecRLE::data CodecRLE::GetData(const std::u32string& inputStr)
{
    std::queue<std::pair<int8_t, std::u32string>> encodedStr;

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::u32string uniqueSeq(1, inputStr[0]); // last sequence of unique characters
    bool flag = false; // show if previous character was part of sequence
    char32_t prev = inputStr[0]; // previous character

    int maxPossibleNumber = 127; // maximum possible value of int8_t
    uniqueSeq.reserve(maxPossibleNumber); // optimization

    // start RLE
    for (size_t i = 1; i < inputStr.size(); ++i)
    {
        if (inputStr[i] == prev) 
        {
            // record last sequence of unique symbols if it exists
            if (countUnique > 1) {
                uniqueSeq.pop_back(); // because "prev" was read as unique
                --countUnique; // because "prev" was read as unique

                encodedStr.push(std::make_pair(-countUnique, uniqueSeq));

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { ++countIdent; }
            
            countUnique = 0;
            uniqueSeq.clear();
        }
        else 
        {
            // record last sequence of identical symbols if it exists
            if (countIdent > 1) {
                if (countIdent >= maxPossibleNumber) {
                    for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                        encodedStr.push(std::make_pair(maxPossibleNumber, std::u32string(1, prev)));
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    encodedStr.push(std::make_pair(countIdent % maxPossibleNumber, std::u32string(1, prev)));
                }
                flag = true;
                countIdent = 1;
            } else if (countIdent == 0) {
                countIdent = 1;
            }

            if (flag) {
                countUnique = 1;
                uniqueSeq.clear();
                uniqueSeq.push_back(inputStr[i]);
                flag = false;
            } else {
                if (countUnique == 0) {
                    countUnique = 1;
                    uniqueSeq.clear();
                    uniqueSeq.push_back(prev);
                }

                ++countUnique;
                uniqueSeq.push_back(inputStr[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == maxPossibleNumber) {
                encodedStr.push(std::make_pair(-countUnique, uniqueSeq));
                flag = true;
                countUnique = 0;
                uniqueSeq.clear();
            }
        }
        prev = inputStr[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= maxPossibleNumber) {
            for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                encodedStr.push(std::make_pair(maxPossibleNumber, std::u32string(1, prev)));
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            encodedStr.push(std::make_pair(countIdent % maxPossibleNumber, std::u32string(1, prev)));
        }
    }
    if (countUnique > 0) {
        encodedStr.push(std::make_pair(-countUnique, uniqueSeq));
    }

    return data(inputStr.size(), encodedStr);
}

template <typename valueType>
CodecRLE::data_numerical<valueType> CodecRLE::GetDataNumerical(const std::vector<valueType>& inputNums)
{
    std::queue<std::pair<int8_t, std::vector<valueType>>> encodedNums;

    int countIdent = 1; // current count of repeating identical numbers
    int countUnique = 1; // current count of repeating unique numbers
    std::vector<valueType> uniqueSeq(1, inputNums[0]); // last sequence of unique numbers
    bool flag = false; // show if previous number was part of sequence
    valueType prev = inputNums[0]; // previous number

    int maxPossibleNumber = 127; // maximum possible value of int8_t
    uniqueSeq.reserve(maxPossibleNumber); // optimization

    // start RLE
    for (size_t i = 1; i < inputNums.size(); ++i)
    {
        if (inputNums[i] == prev) 
        {
            // record last sequence of unique numbers if it exists
            if (countUnique > 1) {
                uniqueSeq.pop_back(); // because "prev" was read as unique
                --countUnique; // because "prev" was read as unique

                encodedNums.push(std::make_pair(-countUnique, uniqueSeq));

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { ++countIdent; }
            
            countUnique = 0;
            uniqueSeq.clear();
        }
        else 
        {
            // record last sequence of identical symbols if it exists
            if (countIdent > 1) {
                if (countIdent >= maxPossibleNumber) {
                    for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                        encodedNums.push(std::make_pair(maxPossibleNumber, std::vector<valueType>(1, prev)));
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    encodedNums.push(std::make_pair(countIdent % maxPossibleNumber, std::vector<valueType>(1, prev)));
                }
                flag = true;
                countIdent = 1;
            } else if (countIdent == 0) {
                countIdent = 1;
            }

            if (flag) {
                countUnique = 1;
                uniqueSeq.clear();
                uniqueSeq.push_back(inputNums[i]);
                flag = false;
            } else {
                if (countUnique == 0) {
                    countUnique = 1;
                    uniqueSeq.clear();
                    uniqueSeq.push_back(prev);
                }

                ++countUnique;
                uniqueSeq.push_back(inputNums[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == maxPossibleNumber) {
                encodedNums.push(std::make_pair(-countUnique, uniqueSeq));
                flag = true;
                countUnique = 0;
                uniqueSeq.clear();
            }
        }
        prev = inputNums[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= maxPossibleNumber) {
            for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                encodedNums.push(std::make_pair(maxPossibleNumber, std::vector<valueType>(1, prev)));
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            encodedNums.push(std::make_pair(countIdent % maxPossibleNumber, std::vector<valueType>(1, prev)));
        }
    }
    if (countUnique > 0) {
        encodedNums.push(std::make_pair(-countUnique, uniqueSeq));
    }

    return data_numerical(inputNums.size(), encodedNums);
}

std::string CodecRLE::DecodeRLE(FILE* inputFile)
{
    std::string decodedStr;

    uint64_t strLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    decodedStr.reserve(strLength);

    uint64_t counter = 0;
    int8_t number;
    while (counter < strLength)
    {
        number = FileUtils::ReadValueBinary<int8_t>(inputFile);

        // if starts with negative number
        // (sequence of unqiue symbols)
        if (number < 0)
        {
            for (int8_t i = 0; i < (-number); ++i) {
                decodedStr += CodecUTF8::DecodeChar32FromBinaryFileToString(inputFile);
                ++counter;
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            std::string code = CodecUTF8::DecodeChar32FromBinaryFileToString(inputFile);

            for (int8_t i = 0; i < number; ++i) {
                decodedStr += code;
                ++counter;
            }
        }
    }

    return decodedStr;
}

template <typename valueType>
std::vector<valueType> CodecRLE::DecodeRLENumerical(FILE* inputFile)
{
    std::vector<valueType> decodedNums;

    uint64_t numLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    decodedNums.reserve(numLength);

    uint64_t counter = 0;
    int8_t number;
    while (counter < numLength)
    {
        number = FileUtils::ReadValueBinary<int8_t>(inputFile);

        // if starts with negative number
        // (sequence of unqiue symbols)
        if (number < 0)
        {
            for (int8_t i = 0; i < (-number); ++i) {
                decodedNums.push_back(FileUtils::ReadValueBinary<valueType>(inputFile));
                ++counter;
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            valueType code = FileUtils::ReadValueBinary<valueType>(inputFile);

            for (int8_t i = 0; i < number; ++i) {
                decodedNums.push_back(code);
                ++counter;
            }
        }
    }

    return decodedNums;
}

void CodecRLE::Encode(const char* inputPath, const char* outputPath)
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    data encodingData = GetData(FileUtils::ReadContentToU32String(inputPath));
    FileUtils::AppendValueBinary(outputFile, encodingData.strLength);
    while (!encodingData.encodedStr.empty()) {
        auto elem = encodingData.encodedStr.front();
        FileUtils::AppendValueBinary(outputFile, elem.first);
        CodecUTF8::EncodeString32ToBinaryFile(outputFile, elem.second);
        encodingData.encodedStr.pop();
    }

    FileUtils::CloseFile(outputFile);
}

void CodecRLE::Decode(const char* inputPath, const char* outputPath)
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = DecodeRLE(inputFile);
    FileUtils::AppendStr(outputFile, decodedStr);
    
    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// END IMPLEMENTATION