#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <queue>
#include <map>

#include "FileUtils.h"
#include "CodecUTF8.h"
#include "TextTools.h"

class CodecAC
{
private:
    CodecAC() = default;
public:
    static void Encode(const char* inputPath, const char* outputPath);
    static void Decode(const char* inputPath, const char* outputPath);
protected:
    struct data_local {
        uint8_t alphabetLength;
        std::u32string alphabet;
        std::vector<uint8_t> frequencies;
        uint64_t resultValue;
        data_local(uint8_t _alphabetLength, std::u32string _alphabet, std::vector<uint8_t> _frequencies, uint64_t _resultValue) : alphabetLength(_alphabetLength), alphabet(_alphabet), frequencies(_frequencies), resultValue(_resultValue) {}
    };
    struct data {
        uint64_t strLength;
        std::queue<data_local> queueLocalData;
        data(const uint64_t& _strLength, const std::queue<data_local>& _queueLocalData) : strLength(_strLength), queueLocalData(_queueLocalData) {}
    };

    static data_local Getdata_local(const std::u32string& inputStr);
    static data GetData(const std::u32string& inputStr);
    static std::string DecodeAC(FILE* inputFile);
};


// START IMPLEMENTATION


CodecAC::data_local CodecAC::Getdata_local(const std::u32string& inputStr)
{
    // initialize sorted alphabet and sorted frequencies
    std::u32string alphabet = GetAlphabet(inputStr);
    uint8_t alphabetLength = alphabet.size();
    std::map<char32_t, double> charFrequenciesMap = GetCharFrequenciesMap(alphabet, alphabetLength, inputStr);
    // convert map to vector
    std::vector<std::pair<char32_t, double>> charFrequenciesVector(charFrequenciesMap.begin(), charFrequenciesMap.end());
    // sort vector by frequencies
    std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    // leave in frequencies only 2 digits after the decimal point
    // for correct work of algorithm
    for (auto& elem : charFrequenciesVector) {
        elem.second = static_cast<uint8_t>((elem.second * 100)) / 100.0;
    }

    // inicialize segments
    //// (array of bounds from 0 to 1)
    std::vector<double> segments; segments.reserve(alphabetLength + 1);
    segments.push_back(0.0);
    for (uint8_t i = 1; i < alphabetLength; ++i) {
        segments.push_back(charFrequenciesVector[i - 1].second + segments[i - 1]);
    }
    segments.push_back(1.0);

    // encode (get final left and right bounds)
    double leftBound = 0, rightBound = 1, distance;
    uint8_t index;
    for (char32_t c : inputStr) {
        for (index = 0; index < alphabetLength; ++index) {
            if (charFrequenciesVector[index].first == c) {
                break;
            }
        }

        distance = rightBound - leftBound;
        rightBound = leftBound + segments[index + 1] * distance;
        leftBound = leftBound + segments[index] * distance;
    }

    double resultValueDouble = rightBound / 2 + leftBound / 2;
    // leave only significant fractional part of a number
    uint64_t resultValue = static_cast<uint64_t>(resultValueDouble * 10000000000000000000.0);

    // make alphabet
    std::u32string alphabetLocal;
    for (uint8_t i = 0; i < alphabetLength; ++i) {
        alphabetLocal.push_back(charFrequenciesVector[i].first);
    }

    // make frequencies vector
    std::vector<uint8_t> frequencies;
    for (uint8_t i = 0; i < alphabetLength; ++i) {
        // leave only significant fractional part of a number
        frequencies.push_back(static_cast<uint8_t>(charFrequenciesVector[i].second * 100));
    }

    return data_local(alphabetLength, alphabetLocal, frequencies, resultValue);
}

CodecAC::data CodecAC::GetData(const std::u32string& inputStr)
{
    // maximum number of character in the string to make encoding 
    const uint8_t numChars = 13;
    std::queue<data_local> queueLocalData; 

    uint64_t strLength = inputStr.size();
    // number of sequences to encode
    uint64_t numberOfFullSequences = (strLength % numChars == 0) ? (strLength / numChars) : (strLength / numChars);

    // encode every <numChars> characters
    uint64_t CountOfSeq = 0;
    while (CountOfSeq < numberOfFullSequences) {
        queueLocalData.push(Getdata_local(inputStr.substr(CountOfSeq * numChars, numChars)));
        ++CountOfSeq;
    }
    // handle the rest of the string
    if (strLength % numChars != 0) {
        queueLocalData.push(Getdata_local(inputStr.substr(numberOfFullSequences * numChars, strLength % numChars)));
    }

    return data(strLength, queueLocalData);
}

std::string CodecAC::DecodeAC(FILE* inputFile)
{
    uint64_t strLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    // maximum number of character in the string to make encoding 
    const uint8_t numChars = 13;
    // number of sequences to encode
    uint64_t numberOfFullSequences = (strLength % numChars == 0) ? (strLength / numChars) : (strLength / numChars);

    // counter of decoded sequences
    uint64_t CountOfSeq = 0;
    std::string result;

    uint8_t alphabetLength;
    std::u32string alphabet;
    std::u32string result_local; result_local.reserve(numChars);
    double resultValue;
    while (CountOfSeq < numberOfFullSequences) {
        // read values
        alphabetLength = FileUtils::ReadValueBinary<uint8_t>(inputFile);
        alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);
        std::vector<double> frequencies; frequencies.reserve(alphabetLength);
        for (uint8_t i = 0; i < alphabetLength; ++i) {
            frequencies.push_back(FileUtils::ReadValueBinary<uint8_t>(inputFile) / 100.0);
        }

        resultValue = FileUtils::ReadValueBinary<uint64_t>(inputFile) / 10000000000000000000.0;

        // decoding
        // inicialize segments
        std::vector<double> segments; segments.reserve(alphabetLength + 1);
        segments.push_back(0.0);
        for (uint32_t i = 1; i < alphabetLength; ++i) {
            segments.push_back(frequencies[i - 1] + segments[i - 1]);
        }
        segments.push_back(1.0);

        // decode
        double leftBound = 0, rightBound = 1, distance;
        int index;
        for (uint8_t i = 0; i < numChars; ++i) {
            // find index of segment contains resultValue
            for (uint8_t j = 0; j < alphabetLength; ++j) {
                if (resultValue >= (leftBound + segments[j] * (rightBound - leftBound)) && 
                    resultValue < (leftBound + segments[j + 1] * (rightBound - leftBound))) {
                    index = j;
                    break;
                }
            }
            result_local.push_back(alphabet[index]);

            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        result += CodecUTF8::EncodeString32ToString(result_local);
        result_local.clear();
        ++CountOfSeq;
    }
    // handle the rest
    // (same code but with different count of iterations in while loop)
    if (strLength % numChars != 0) {
        // read values
        alphabetLength = FileUtils::ReadValueBinary<uint8_t>(inputFile);
        alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);
        std::vector<double> frequencies; frequencies.reserve(alphabetLength);
        for (uint8_t i = 0; i < alphabetLength; ++i) {
            frequencies.push_back(FileUtils::ReadValueBinary<uint8_t>(inputFile) / 100.0);
        }
        resultValue = FileUtils::ReadValueBinary<uint64_t>(inputFile) / 10000000000000000000.0;

        // decoding
        // inicialize segments
        std::vector<double> segments; segments.reserve(alphabetLength + 1);
        segments.push_back(0);
        for (uint32_t i = 1; i < alphabetLength; ++i) {
            segments.push_back(frequencies[i - 1] + segments[i - 1]);
        }
        segments.push_back(1);

        // decode
        double leftBound = 0, rightBound = 1, distance;
        int index;
        for (uint8_t i = 0; i < strLength % numChars; ++i) {
            // find index of segment contains resultValue
            for (uint8_t j = 0; j < alphabetLength; ++j) {
                if (resultValue >= (leftBound + segments[j] * (rightBound - leftBound)) && 
                    resultValue < (leftBound + segments[j + 1] * (rightBound - leftBound))) {
                    index = j;
                    break;
                }
            }
            result_local.push_back(alphabet[index]);

            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        result += CodecUTF8::EncodeString32ToString(result_local);
        ++CountOfSeq;
    }

    return result;
}

void CodecAC::Encode(const char* inputPath, const char* outputPath)
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    data encodingData = GetData(FileUtils::ReadContentToU32String(inputPath));
    FileUtils::AppendValueBinary(outputFile, encodingData.strLength);

    while (!encodingData.queueLocalData.empty()) {
        auto elem = encodingData.queueLocalData.front();

        FileUtils::AppendValueBinary(outputFile, elem.alphabetLength);
        CodecUTF8::EncodeString32ToBinaryFile(outputFile, elem.alphabet);
        for (uint8_t i = 0; i < elem.alphabetLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, elem.frequencies[i]);
        }
        FileUtils::AppendValueBinary(outputFile, elem.resultValue);

        encodingData.queueLocalData.pop();
    }
    FileUtils::CloseFile(outputFile);
}

void CodecAC::Decode(const char* inputPath, const char* outputPath)
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = DecodeAC(inputFile);
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// END IMPLEMENTATION