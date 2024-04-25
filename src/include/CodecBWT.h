#pragma once

#include <string>
#include <cstdint>

#include "FileUtils.h"
#include "CodecUTF8.h"
#include "SuffixArray.h"

class CodecBWT
{
private:
    CodecBWT() = default;
public:
    static void Encode(const char* inputPath, const char* outputPath);
    static void Decode(const char* inputPath, const char* outputPath);
protected:
    struct data {
        uint32_t index;
        std::u32string encodedStr;
        data(const uint32_t& _index, const std::u32string& _encodedStr) : index(_index), encodedStr(_encodedStr) {}
    };

    static data GetData(const std::u32string& inputStr);
    static std::u32string DecodeBWT(FILE* inputFile);
};


// START IMPLEMENTATION


CodecBWT::data CodecBWT::GetData(const std::u32string& inputStr)
{
    // NOTE:
    // will encode every 10 * 1024 * 1024 (10 Mb in the worst case, else even more Mb)
    // of characaters of inputStr.
    // then about 100 Mb of RAM will be used
    // except of the queueLocalData which use 4 * |text| in the worst case,
    // where |text| is the size of the file with the input string
    // so enwik8 will use about 500mb of RAM
    //const size_t MAX_COUNT_OF_CHARS = 10 * 1024 * 1024;

    uint32_t index;
    std::u32string encodedStr; encodedStr.reserve(inputStr.size());
    std::vector<unsigned int> suffixArray = buildSuffixArray(inputStr);
    for (size_t i = 0; i < suffixArray.size(); ++i) {
        size_t ind = (suffixArray[i] > 0) ? (suffixArray[i] - 1) : (inputStr.size() - 1);
        encodedStr.push_back(inputStr[ind]);
        if (suffixArray[i] == 0) {
            index = i;
        }
    }

    return data(index, encodedStr);
}

std::u32string CodecBWT::DecodeBWT(FILE* inputFile)
{
    uint32_t index = FileUtils::ReadValueBinary<uint32_t>(inputFile);
    uint64_t strSize = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    std::u32string inputStr = CodecUTF8::DecodeString32FromBinaryFile(inputFile, strSize);

    std::vector<std::pair<char32_t, unsigned int>> P; P.reserve(inputStr.size());
    for (uint64_t i = 0; i < strSize; ++i) {
        P.push_back(std::make_pair(inputStr[i], i));
    }
    std::sort(P.begin(), P.end());

    std::u32string decodedStr; decodedStr.reserve(inputStr.size());
    for (size_t i = 0; i < inputStr.size(); ++i) {
        index = P[index].second;
        decodedStr.push_back(inputStr[index]);
    }

    return decodedStr;
}

void CodecBWT::Encode(const char* inputPath, const char* outputPath)
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    data encodingData = GetData(FileUtils::ReadContentToU32String(inputPath));
    FileUtils::AppendValueBinary(outputFile, encodingData.index);
    FileUtils::AppendValueBinary(outputFile, static_cast<uint64_t>(encodingData.encodedStr.size()));
    CodecUTF8::EncodeString32ToBinaryFile(outputFile, encodingData.encodedStr);

    FileUtils::CloseFile(outputFile);
}

void CodecBWT::Decode(const char* inputPath, const char* outputPath)
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = CodecUTF8::EncodeString32ToString(DecodeBWT(inputFile));
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// END IMPLEMENTATION