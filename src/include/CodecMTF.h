#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "FileUtils.h"
#include "CodecUTF8.h"
#include "TextTools.h"

class CodecMTF
{
private:
    CodecMTF() = default;
public:
    static void Encode(const char* inputPath, const char* outputPath);
    static void Decode(const char* inputPath, const char* outputPath);
protected:
    struct data {
        std::u32string alphabet;
        uint64_t strLength;
        std::vector<uint32_t> codes;
        data(std::u32string _alphabet, uint64_t _strLength, std::vector<uint32_t> _codes) : alphabet(_alphabet), strLength(_strLength), codes(_codes) {}
    };

    template <typename valueType>
    static void AlphabetShift(std::u32string& alphabet, const valueType& index);
    static const uint32_t GetIndex(const std::u32string& alphabet, const char32_t c);
    static data GetData(const std::u32string& inputStr);
    static std::string DecodeMTF(FILE* inputFile);
};


// START IMPLEMENTATION

template <typename valueType>
void CodecMTF::AlphabetShift(std::u32string& alphabet, const valueType& index)
{
    char32_t temp = alphabet[0], temp2;
    for (valueType i = 1; i <= index; ++i) {
        temp2 = alphabet[i];
        alphabet[i] = temp;
        temp = temp2;
    }
    alphabet[0] = temp;
}

const uint32_t CodecMTF::GetIndex(const std::u32string& alphabet, const char32_t c)
{
    for (size_t i = 0; i < alphabet.size(); ++i) {
        if (alphabet[i] == c) {
            return static_cast<uint32_t>(i);
        }
    }
    return 0; // assume that this will never happen
}

CodecMTF::data CodecMTF::GetData(const std::u32string& inputStr)
{
    std::u32string alphabet = GetAlphabet(inputStr);
    uint64_t strLength = inputStr.size();

    std::vector<uint32_t> codes; codes.reserve(strLength);
    uint32_t index;
    // move-to-front
    for (uint64_t i = 0; i < strLength; ++i) {
        index = GetIndex(alphabet, inputStr[i]);
        codes.push_back(index);

        AlphabetShift(alphabet, index);
    }

    std::sort(alphabet.begin(), alphabet.end());
    return data(alphabet, strLength, codes);
}

std::string CodecMTF::DecodeMTF(FILE* inputFile)
{
    uint32_t alphabetLength = FileUtils::ReadValueBinary<uint32_t>(inputFile);
    std::u32string alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);
    uint64_t strLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);

    std::u32string decodedStr; decodedStr.reserve(strLength);
    char32_t temp, temp2;

    // decode
    if (alphabetLength <= 256) {
        uint8_t index;
        for (uint64_t i = 0; i < strLength; ++i) {
            index = FileUtils::ReadValueBinary<uint8_t>(inputFile);
            decodedStr.push_back(alphabet[index]);

            AlphabetShift(alphabet, index);
        }
    } else if (alphabetLength <= 65536) {
        uint16_t index;
        for (uint64_t i = 0; i < strLength; ++i) {
            index = FileUtils::ReadValueBinary<uint16_t>(inputFile);
            decodedStr.push_back(alphabet[index]);

            AlphabetShift(alphabet, index);
        }
    } else {
        uint32_t index;
        for (uint64_t i = 0; i < strLength; ++i) {
            index = FileUtils::ReadValueBinary<uint32_t>(inputFile);
            decodedStr.push_back(alphabet[index]);

            AlphabetShift(alphabet, index);
        }
    }

    return CodecUTF8::EncodeString32ToString(decodedStr);
}

void CodecMTF::Encode(const char* inputPath, const char* outputPath)
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    data encodingData = GetData(FileUtils::ReadContentToU32String(inputPath));
    FileUtils::AppendValueBinary(outputFile, static_cast<uint32_t>(encodingData.alphabet.size()));
    CodecUTF8::EncodeString32ToBinaryFile(outputFile, encodingData.alphabet);
    FileUtils::AppendValueBinary(outputFile, encodingData.strLength);

    if (encodingData.alphabet.size() <= 256) {
        for (uint64_t i = 0; i < encodingData.strLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint8_t>(encodingData.codes[i]));
        }
    } else if (encodingData.alphabet.size() <= 65536) {
        for (uint64_t i = 0; i < encodingData.strLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint16_t>(encodingData.codes[i]));
        }
    } else {
        for (uint64_t i = 0; i < encodingData.strLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, encodingData.codes[i]);
        }
    }
    FileUtils::CloseFile(outputFile);
}

void CodecMTF::Decode(const char* inputPath, const char* outputPath)
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = DecodeMTF(inputFile);
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// END IMPLEMENTATION