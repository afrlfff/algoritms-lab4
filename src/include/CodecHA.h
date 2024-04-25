#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <queue>

#include "FileUtils.h"
#include "CodecUTF8.h"
#include "HuffmanTree.h"
#include "TextTools.h"


class CodecHA
{
private:
    CodecHA() = default;
public:
    static void Encode(const char* inputPath, const char* outputPath);
    static void Decode(const char* inputPath, const char* outputPath);
protected:
    struct data_local {
        uint8_t alphabetLength;
        std::u32string alphabet;
        std::map<char32_t, std::string> huffmanCodesMap;
        std::string encodedStr;
        data_local(const uint16_t& _alphabetLength, const std::u32string& _alphabet, const std::map<char32_t, std::string>& _huffmanCodesMap, const std::string& _encodedStr) : alphabetLength(_alphabetLength), alphabet(_alphabet), huffmanCodesMap(_huffmanCodesMap), encodedStr(_encodedStr) {}
    };
    struct data {
        std::queue<data_local> queueLocalData;
        data(const std::queue<data_local>& _queueLocalData) : queueLocalData(_queueLocalData) {}
    };

    static uint8_t GetNumberFromBinaryString(const std::string& binaryString);
    static std::string GetBinaryStringFromNumber(const uint8_t& number, const uint8_t& codeLength);

    static data GetData(const std::u32string& inputStr);
    static std::u32string DecodeHA(FILE* inputFile);
};


// START IMPLEMENTATION


uint8_t CodecHA::GetNumberFromBinaryString(const std::string& binaryString)
{
    uint8_t result = 0;
    for (size_t i = 0; i < binaryString.size(); ++i) {
        if (binaryString[i] == '1') {
            result |= (1 << (binaryString.length() - i - 1));
        }
    }
    return result;
}

std::string CodecHA::GetBinaryStringFromNumber(const uint8_t& number, const uint8_t& codeLength)
{
    std::string result(codeLength, '0');
    for (uint8_t i = 0; i < codeLength; ++i) {
        if (number & (1 << (codeLength - i - 1))) {
            result[i] = '1';
        }
    }
    return result;
} 

CodecHA::data CodecHA::GetData(const std::u32string& inputStr)
{
    std::queue<data_local> queueLocalData;

    const size_t strLengthToStart = 50;
    const size_t strLengthToAppend = 10;
    const size_t maxHuffmanCodeLength = 8;
    uint8_t maxHuffmanCodeLengthCounter;
    size_t stringPointer = 0;
    std::u32string localString;

    std::set<char32_t> alphabetSet;
    std::map<char32_t, size_t> charCountsMap;
    std::vector<std::pair<char32_t, double>> charFrequenciesVector;


    // get all the data_local
    while (stringPointer < inputStr.size()) {
        // inicialization
        localString = inputStr.substr(stringPointer, strLengthToStart);
        alphabetSet = GetAlphabetSet(localString);
        charCountsMap = GetCharCountsMap(std::u32string(alphabetSet.begin(), alphabetSet.end()), alphabetSet.size(), localString);
        charFrequenciesVector = std::vector<std::pair<char32_t, double>>(charCountsMap.begin(), charCountsMap.end());
        for (size_t i = 0; i < charFrequenciesVector.size(); ++i) {
            charFrequenciesVector[i].second = static_cast<double>(charFrequenciesVector[i].second) / alphabetSet.size();
        }
        // sort vector by frequencies
        std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
        

        while (true) {
            // get maxHuffmanCodeLengthCounter
            HuffmanTree tree = BuildHuffmanTree(charFrequenciesVector, alphabetSet.size());
            maxHuffmanCodeLengthCounter = tree.GetHeight();
            if ((maxHuffmanCodeLengthCounter > maxHuffmanCodeLength) || 
                (alphabetSet.size() > 256)) {
                // reset data to the last modification where maxHuffmanCodeLengthCounter <= maxHuffmanCodeLength

                localString.erase(localString.size() - strLengthToAppend); // remove last 10 characters
                alphabetSet = GetAlphabetSet(localString);
                charCountsMap = GetCharCountsMap(std::u32string(alphabetSet.begin(), alphabetSet.end()), alphabetSet.size(), localString);
                charFrequenciesVector = std::vector<std::pair<char32_t, double>>(charCountsMap.begin(), charCountsMap.end());
                for (size_t i = 0; i < charFrequenciesVector.size(); ++i) {
                    charFrequenciesVector[i].second = static_cast<double>(charFrequenciesVector[i].second) / alphabetSet.size();
                }
                // sort vector by frequencies
                std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
                    return a.second < b.second;
                });

                break;
            } else if (stringPointer + localString.size() >= inputStr.size()) {
                break;
            }

            // modify the data
            // expand localString
            localString += inputStr.substr(stringPointer + localString.size(), strLengthToAppend);
            // expand alphabetSet and charCountsMap
            for (size_t i = localString.size() - strLengthToAppend; i < localString.size(); ++i) {
                alphabetSet.insert(localString[i]);
                ++charCountsMap[localString[i]];
            }
            // get charFrequenciesVector
            charFrequenciesVector = std::vector<std::pair<char32_t, double>>(charCountsMap.begin(), charCountsMap.end());
            for (size_t i = 0; i < charFrequenciesVector.size(); ++i) {
                charFrequenciesVector[i].second = static_cast<double>(charFrequenciesVector[i].second) / alphabetSet.size();
            }
            // sort vector by frequencies
            std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        }

        
        HuffmanTree tree = BuildHuffmanTree(charFrequenciesVector, alphabetSet.size());
        std::map<char32_t, std::string> huffmanCodesMap = GetHuffmanCodes(tree, alphabetSet.size());
        std::string encodedStr;
        // !!!!!
        // AM I RIGHT THAT I SHOULD ENCODE LOCAL STRING?
        for (size_t i = 0; i < localString.size(); ++i) {
            encodedStr += huffmanCodesMap[localString[i]];
        }
        queueLocalData.push(data_local(alphabetSet.size(), std::u32string(alphabetSet.begin(), alphabetSet.end()), huffmanCodesMap, encodedStr));
        
        // print to see compression ratio of huffman result sequence
        //std::cout << "local part: " << localString.size() << " -> "
        //          << encodedStr.size() / static_cast<double>(8) << std::endl;
        
        // move stringPointer
        stringPointer += localString.size();
    }
    
    return data(queueLocalData);
}

std::u32string CodecHA::DecodeHA(FILE* inputFile)
{
    uint64_t numberOfLocalData = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    std::u32string decodedStr;

    for (uint64_t i = 0; i < numberOfLocalData; ++i) {
        uint8_t alphabetLength = FileUtils::ReadValueBinary<uint8_t>(inputFile);
        std::u32string alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);

        uint8_t lengthsOfCodesSize = alphabetLength;
        std::string lengthsOfCodes = FileUtils::ReadSequenceOfDigitsBinary(inputFile, lengthsOfCodesSize);

        // get length of all the huffman codes
        uint16_t allHuffmanCodesSize = 0;
        for (char c : lengthsOfCodes) {
            allHuffmanCodesSize += c - '0';
        }
        // read string of encoded huffman codes
        std::string allHuffmanCodes;
        for (size_t i = 0; i < allHuffmanCodesSize - allHuffmanCodesSize % 8; i += 8) {
            allHuffmanCodes += GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), 8);
        }
        if (allHuffmanCodesSize % 8 != 0) {
            allHuffmanCodes += GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), allHuffmanCodesSize % 8);
        }
        // get huffman codes map
        std::map<std::string, char32_t> huffmanCodesMap;
        size_t stringPointer = 0;
        for (uint8_t j = 0; j < alphabetLength; ++j) {
            huffmanCodesMap[allHuffmanCodes.substr(stringPointer, lengthsOfCodes[j] - '0')] = alphabet[j];
            stringPointer += lengthsOfCodes[j] - '0';
        }

        // read encoded string
        uint64_t encodedStrLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
        std::string encodedStr; encodedStr.reserve(encodedStrLength);
        for (uint64_t j = 0; j < encodedStrLength - encodedStrLength % 8; j += 8) {
            encodedStr += GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), 8);
        }
        if (encodedStrLength % 8 != 0) {
            encodedStr += GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), encodedStrLength % 8);
        }

        // decode encoded string
        std::u32string decodedStrLocal;
        std::string currentCode; currentCode.reserve(8);
        for (size_t j = 0; j < encodedStr.size(); ++j) {
            currentCode.push_back(encodedStr[j]);
            if (huffmanCodesMap.find(currentCode) != huffmanCodesMap.end()) {
                decodedStrLocal.push_back(huffmanCodesMap[currentCode]);
                currentCode.clear();
            }
        }

        decodedStr += decodedStrLocal;
    }

    return decodedStr;
}

void CodecHA::Encode(const char* inputPath, const char* outputPath)
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    data encodingData = GetData(FileUtils::ReadContentToU32String(inputPath));
    
    FileUtils::AppendValueBinary(outputFile, static_cast<uint64_t>(encodingData.queueLocalData.size()));
    while (!encodingData.queueLocalData.empty()) {
        data_local dataLocal = encodingData.queueLocalData.front();

        FileUtils::AppendValueBinary(outputFile, dataLocal.alphabetLength);
        CodecUTF8::EncodeString32ToBinaryFile(outputFile, dataLocal.alphabet);

        std::map<char32_t, std::string> huffmanCodesMap = dataLocal.huffmanCodesMap;
        // write lengths of huffman codes effectively
        std::string lengthsOfCodes;
        for (size_t i = 0; i < dataLocal.alphabetLength; ++i) {
            lengthsOfCodes.push_back(huffmanCodesMap[dataLocal.alphabet[i]].size() + '0');
        }
        FileUtils::AppendSequenceOfDigitsBinary(outputFile, lengthsOfCodes);
        // write huffman codes as a single string effectively
        std::string allHuffmanCodes;
        for (size_t i = 0; i < dataLocal.alphabetLength; ++i) {
            allHuffmanCodes += huffmanCodesMap[dataLocal.alphabet[i]];
        }
        for (size_t i = 0; i < allHuffmanCodes.size() - allHuffmanCodes.size() % 8; i += 8) {
            FileUtils::AppendValueBinary(outputFile, GetNumberFromBinaryString(allHuffmanCodes.substr(i, 8)));
        }
        if (allHuffmanCodes.size() % 8 != 0) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint8_t>(GetNumberFromBinaryString(allHuffmanCodes.substr(allHuffmanCodes.size() - allHuffmanCodes.size() % 8))));
        }
        
        // write encoded string effectively
        std::string encodedStr = dataLocal.encodedStr;
        FileUtils::AppendValueBinary(outputFile, static_cast<uint64_t>(encodedStr.size()));
        for (size_t i = 0; i < encodedStr.size() - encodedStr.size() % 8; i += 8) {
            FileUtils::AppendValueBinary(outputFile, GetNumberFromBinaryString(encodedStr.substr(i, 8)));
        }
        if (encodedStr.size() % 8 != 0) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint8_t>(GetNumberFromBinaryString(encodedStr.substr(encodedStr.size() - encodedStr.size() % 8))));
        }

        encodingData.queueLocalData.pop();
    }

    FileUtils::CloseFile(outputFile);
}

void CodecHA::Decode(const char* inputPath, const char* outputPath)
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = CodecUTF8::EncodeString32ToString(DecodeHA(inputFile));
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// END IMPLEMENTATION