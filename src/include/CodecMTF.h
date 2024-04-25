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
        uint32_t alphabetLength;
        std::u32string alphabet;
        uint64_t strLength;
        std::vector<uint32_t> codes;
        data(uint32_t _alphabetLength, std::u32string _alphabet, uint64_t _strLength, std::vector<uint32_t> _codes) : alphabetLength(_alphabetLength), alphabet(_alphabet), strLength(_strLength), codes(_codes) {}
    };

    static void AlphabetShift(std::u32string& alphabet, const uint16_t& alphabetLength, const uint32_t& index);
    static const uint32_t GetIndex(const std::u32string& alphabet, const uint16_t alphabetLength, const char32_t c);
    static data GetData(const std::u32string& inputStr);
    static std::string DecodeMTF(FILE* inputFile);
};


// START IMPLEMENTATION


void CodecMTF::AlphabetShift(std::u32string& alphabet, const uint16_t& alphabetLength, const uint32_t& index)
{
    char32_t temp = alphabet[0], temp2;
    for (uint32_t j = 1; j <= index; ++j) {
        temp2 = alphabet[j];
        alphabet[j] = temp;
        temp = temp2;
    }
    alphabet[0] = temp;
}

const uint32_t CodecMTF::GetIndex(const std::u32string& alphabet, const uint16_t alphabetLength, const char32_t c)
{
    for (uint16_t i = 0; i < alphabetLength; ++i) {
        if (alphabet[i] == c) {
            return i;
        }
    }
    return 0; // assume that this will never happen
}

CodecMTF::data CodecMTF::GetData(const std::u32string& inputStr)
{
    std::u32string alphabet = GetAlphabet(inputStr);
    uint32_t alphabetLength = alphabet.size();
    uint64_t strLength = inputStr.size();

    std::vector<uint32_t> codes; codes.reserve(strLength);
    // move-to-front
    for (uint64_t i = 0; i < strLength; ++i) {
        uint32_t index = GetIndex(alphabet, alphabetLength, inputStr[i]);
        codes.push_back(index);

        AlphabetShift(alphabet, alphabetLength, index);
    }

    std::sort(alphabet.begin(), alphabet.end());
    return data(alphabetLength, alphabet, strLength, codes);
}

std::string CodecMTF::DecodeMTF(FILE* inputFile)
{
    // read meta data
    uint32_t alphabetLength = FileUtils::ReadValueBinary<uint32_t>(inputFile);
    //std::u32string alphabet = FileUtils::ReadStrBinary<stringType, equalCharType>(inputFile, alphabetLength);
    std::u32string alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);
    uint64_t strLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);

    // decode
    std::u32string decodedStr; decodedStr.reserve(strLength);
    uint16_t index;
    char32_t temp, temp2;

    if (alphabetLength <= 256) {
        for (uint64_t i = 0; i < strLength; ++i) {
            index = FileUtils::ReadValueBinary<uint8_t>(inputFile);
            decodedStr.push_back(alphabet[index]);

            AlphabetShift(alphabet, alphabetLength, index);
        }
    } else if (alphabetLength <= 65536) {
        for (uint64_t i = 0; i < strLength; ++i) {
            index = FileUtils::ReadValueBinary<uint16_t>(inputFile);
            decodedStr.push_back(alphabet[index]);

            AlphabetShift(alphabet, alphabetLength, index);
        }
    } else {
        for (uint64_t i = 0; i < strLength; ++i) {
            index = FileUtils::ReadValueBinary<uint32_t>(inputFile);
            decodedStr.push_back(alphabet[index]);

            AlphabetShift(alphabet, alphabetLength, index);
        }
    }

    return CodecUTF8::EncodeString32ToString(decodedStr);
}

void CodecMTF::Encode(const char* inputPath, const char* outputPath)
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    data encodingData = GetData(FileUtils::ReadContentToU32String(inputPath));
    FileUtils::AppendValueBinary(outputFile, encodingData.alphabetLength);
    CodecUTF8::EncodeString32ToBinaryFile(outputFile, encodingData.alphabet);
    FileUtils::AppendValueBinary(outputFile, encodingData.strLength);

    if (encodingData.alphabetLength <= 256) {
        for (uint64_t i = 0; i < encodingData.strLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint8_t>(encodingData.codes[i]));
        }
    } else if (encodingData.alphabetLength <= 65536) {
        for (uint64_t i = 0; i < encodingData.strLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint16_t>(encodingData.codes[i]));
        }
    } else {
        for (uint64_t i = 0; i < encodingData.strLength; ++i) {
            FileUtils::AppendValueBinary(outputFile, static_cast<uint32_t>(encodingData.codes[i]));
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