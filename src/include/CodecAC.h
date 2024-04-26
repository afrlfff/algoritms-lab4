#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <queue>
#include <map>
#include <cmath> // for std::trunc

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
    std::map<char32_t, double> charFrequenciesMap = GetCharFrequenciesMap(alphabet, alphabet.size(), inputStr);
    // convert map to vector
    std::vector<std::pair<char32_t, double>> charFrequenciesVector(charFrequenciesMap.begin(), charFrequenciesMap.end());
    // sort vector by frequencies
    std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    // leave in frequencies only 2 digits after the decimal point
    for (auto& elem : charFrequenciesVector) {
        elem.second = std::trunc((elem.second * 100.0)) / 100.0;
    }

    // inicialize segments
    //// (array of bounds from 0 to 1)
    std::vector<double> segments; segments.reserve(alphabet.size() + 1);
    segments.push_back(0.0);
    for (size_t i = 1; i < alphabet.size(); ++i) {
        segments.push_back(charFrequenciesVector[i - 1].second + segments[i - 1]);
    }
    segments.push_back(1.0);

    // encode (get final left and right bounds)
    long double leftBound = 0, rightBound = 1, distance;
    size_t index;
    for (char32_t c : inputStr) {
        for (index = 0; index < alphabet.size(); ++index) {
            if (charFrequenciesVector[index].first == c) {
                break;
            }
        }

        distance = rightBound - leftBound;
        rightBound = leftBound + segments[index + 1] * distance;
        leftBound = leftBound + segments[index] * distance;
    }

    long double resultValueDouble = rightBound / (long double)(2.0) + leftBound / (long double)(2.0);
    // leave only significant fractional part of a number
    uint64_t resultValue = resultValueDouble * 1e17;

    // make alphabet
    std::u32string alphabetLocal;
    for (size_t i = 0; i < alphabet.size(); ++i) {
        alphabetLocal.push_back(charFrequenciesVector[i].first);
    }

    // make frequencies vector
    std::vector<uint8_t> frequencies;
    for (size_t i = 0; i < alphabet.size(); ++i) {
        // leave only significant fractional part of a number
        frequencies.push_back(static_cast<uint8_t>(charFrequenciesVector[i].second * 100.0));
    }

    return data_local(alphabetLocal.size(), alphabetLocal, frequencies, resultValue);
}

CodecAC::data CodecAC::GetData(const std::u32string& inputStr)
{
    // maximum number of character in the string to make encoding 
    const uint8_t numChars = 14;
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
    // maximum number of character in the string to make local encoding 
    const uint8_t numChars = 14;

    uint64_t strLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    // number of sequences to encode
    uint64_t numberOfFullSequences = (strLength % numChars == 0) ? (strLength / numChars) : (strLength / numChars);

    // counter of decoded sequences
    uint64_t seqsCounter = 0;
    std::string result;

    uint8_t alphabetLength;
    std::u32string alphabet;
    std::u32string result_local; result_local.reserve(numChars);
    long double resultValue;
    while (seqsCounter < numberOfFullSequences) {
        // read values
        alphabetLength = FileUtils::ReadValueBinary<uint8_t>(inputFile);
        alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);
        std::vector<double> frequencies; frequencies.reserve(alphabetLength);
        for (uint8_t i = 0; i < alphabetLength; ++i) {
            frequencies.push_back(static_cast<double>(FileUtils::ReadValueBinary<uint8_t>(inputFile)) / 100.0);
        }

        resultValue = static_cast<long double>(FileUtils::ReadValueBinary<uint64_t>(inputFile)) / static_cast<long double>(1e17);

        // decoding
        // inicialize segments
        std::vector<double> segments; segments.reserve(alphabet.size() + 1);
        segments.push_back(0.0);
        for (size_t i = 1; i < alphabet.size(); ++i) {
            segments.push_back(frequencies[i - 1] + segments[i - 1]);
        }
        segments.push_back(1.0);

        // decode
        long double leftBound = 0, rightBound = 1, distance;
        size_t index;
        for (uint8_t i = 0; i < numChars; ++i) {
            // find index of segment contains resultValue
            for (size_t j = 0; j < alphabet.size(); ++j) {
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
        ++seqsCounter;
    }
    // handle the rest
    // (same code but with different count of iterations in while loop)
    if (strLength % numChars != 0) {
        // read values
        alphabetLength = FileUtils::ReadValueBinary<uint8_t>(inputFile);
        alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);
        std::vector<double> frequencies; frequencies.reserve(alphabetLength);
        for (uint8_t i = 0; i < alphabetLength; ++i) {
            frequencies.push_back(static_cast<double>(FileUtils::ReadValueBinary<uint8_t>(inputFile)) / 100.0);
        }
        resultValue = static_cast<long double>(FileUtils::ReadValueBinary<uint64_t>(inputFile)) / static_cast<long double>(1e17);

        // decoding
        // inicialize segments
        std::vector<double> segments; segments.reserve(alphabet.size() + 1);
        segments.push_back(0.0);
        for (size_t i = 1; i < alphabet.size(); ++i) {
            segments.push_back(frequencies[i - 1] + segments[i - 1]);
        }
        segments.push_back(1.0);

        // decode
        long double leftBound = 0, rightBound = 1, distance;
        size_t index;
        for (uint64_t i = 0; i < strLength % numChars; ++i) {
            // find index of segment contains resultValue
            for (size_t j = 0; j < alphabet.size(); ++j) {
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
        ++seqsCounter;
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