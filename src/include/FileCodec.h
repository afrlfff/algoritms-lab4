#pragma once

#include <string>
#include <set> // makes ordered set
#include <queue>
#include <algorithm> // sorting
#include <cstdint> // for int8_t, int16_t ...
#include <utility> // for std::pair

#include "FileUtils.h"
#include "CodecUTF8.h"

#include "TextTools.h"
#include "HuffmanTree.h"


class FileCodec
{
public:
    virtual void Encode(const char* inputPath, const char* outputPath) const = 0; 
    virtual void Decode(const char* inputPath, const char* outputPath) const = 0;
protected:
    FileCodec() = default;
};

// Run-length encoding
class CodecRLE : public FileCodec
{
public:
    CodecRLE() = default;
    void Encode(const char* inputPath, const char* outputPath) const override;
    void Decode(const char* inputPath, const char* outputPath) const override;
protected:
    struct dataRLE {
        uint64_t strLength;
        std::queue<std::pair<int8_t, std::u32string>> encodedStr;
        dataRLE(size_t _strLength, std::queue<std::pair<int8_t, std::u32string>> _encodedStr) : strLength(_strLength), encodedStr(_encodedStr) {}
    };

    dataRLE GetDataRLE(const std::u32string& inputStr) const;
    std::string DecodeRLE(FILE* inputFile) const;
};

// Move-to-front
class CodecMTF : public FileCodec
{
public:
    CodecMTF() = default;
    void Encode(const char* inputPath, const char* outputPath) const override;
    void Decode(const char* inputPath, const char* outputPath) const override;
protected:
    struct dataMTF {
        uint32_t alphabetLength;
        std::u32string alphabet;
        uint64_t strLength;
        std::vector<uint32_t> codes;
        dataMTF(uint32_t _alphabetLength, std::u32string _alphabet, uint64_t _strLength, std::vector<uint32_t> _codes) : alphabetLength(_alphabetLength), alphabet(_alphabet), strLength(_strLength), codes(_codes) {}
    };

    void AlphabetShift(std::u32string& alphabet, const uint16_t& alphabetLength, const uint32_t& index) const;
    const uint32_t GetIndex(const std::u32string& alphabet, const uint16_t alphabetLength, const char32_t c) const;
    dataMTF GetDataMTF(const std::u32string& inputStr) const;
    std::string DecodeMTF(FILE* inputFile) const;
};

// Ariphmetical encoding
class CodecAC : public FileCodec
{
public:
    CodecAC() = default;
    void Encode(const char* inputPath, const char* outputPath) const override;
    void Decode(const char* inputPath, const char* outputPath) const override;
protected:
    struct dataAC_local {
        uint8_t alphabetLength;
        std::u32string alphabet;
        std::vector<uint8_t> frequencies;
        uint64_t resultValue;
        dataAC_local(uint8_t _alphabetLength, std::u32string _alphabet, std::vector<uint8_t> _frequencies, uint64_t _resultValue) : alphabetLength(_alphabetLength), alphabet(_alphabet), frequencies(_frequencies), resultValue(_resultValue) {}
    };
    struct dataAC {
        uint64_t strLength;
        std::queue<dataAC_local> queueLocalData;
        dataAC(const uint64_t& _strLength, const std::queue<dataAC_local>& _queueLocalData) : strLength(_strLength), queueLocalData(_queueLocalData) {}
    };

    dataAC_local GetDataAC_local(const std::u32string& inputStr) const;
    dataAC GetDataAC(const std::u32string& inputStr) const;
    std::string DecodeAC(FILE* inputFile) const;
};

// Huffman algorithm encoding
class CodecHA : public FileCodec
{
public:
    CodecHA() = default;
    void Encode(const char* inputPath, const char* outputPath) const override;
    void Decode(const char* inputPath, const char* outputPath) const override;
protected:
    struct dataHA_local {
        uint16_t alphabetLength;
        std::u32string alphabet;
        std::map<char32_t, std::string> huffmanCodesMap;
        std::string encodedStr;
        dataHA_local(const uint16_t& _alphabetLength, const std::u32string& _alphabet, const std::map<char32_t, std::string>& _huffmanCodesMap, const std::string& _encodedStr) : alphabetLength(_alphabetLength), alphabet(_alphabet), huffmanCodesMap(_huffmanCodesMap), encodedStr(_encodedStr) {}
    };
    struct dataHA {
        std::queue<dataHA_local> queueLocalData;
        dataHA(const std::queue<dataHA_local>& _queueLocalData) : queueLocalData(_queueLocalData) {}
    };

    uint8_t GetNumberFromBinaryString(const std::string& binaryString) const;
    std::string GetBinaryStringFromNumber(const uint8_t& number, const uint8_t& codeLength) const;

    dataHA GetDataHA(const std::u32string& inputStr) const;
    std::u32string DecodeHA(FILE* inputFile) const;
};


// START IMPLEMENTATION

// ==================================================================================
// Run-length encoding

CodecRLE::dataRLE CodecRLE::GetDataRLE(const std::u32string& inputStr) const
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

    return dataRLE(inputStr.size(), encodedStr);
}

std::string CodecRLE::DecodeRLE(FILE* inputFile) const
{
    std::string decodedStr;

    uint64_t strLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
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
                //decodedStr.push_back(FileUtils::ReadValueBinary<char32_t>(inputFile));
                decodedStr += CodecUTF8::DecodeChar32FromBinaryFileToString(inputFile);
                ++counter;
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            //char32_t c = FileUtils::ReadValueBinary<char32_t>(inputFile);
            std::string code = CodecUTF8::DecodeChar32FromBinaryFileToString(inputFile);

            for (int8_t i = 0; i < number; ++i) {
                //decodedStr.push_back(c);
                decodedStr += code;
                ++counter;
            }
        }
    }

    return decodedStr;
}

void CodecRLE::Encode(const char* inputPath, const char* outputPath) const
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    dataRLE encodingData = GetDataRLE(FileUtils::ReadContentToU32String(inputPath));
    FileUtils::AppendValueBinary(outputFile, encodingData.strLength);
    while (!encodingData.encodedStr.empty()) {
        auto elem = encodingData.encodedStr.front();
        FileUtils::AppendValueBinary(outputFile, elem.first);
        //FileUtils::AppendStrBinary(outputFile, elem.second);
        CodecUTF8::EncodeString32ToBinaryFile(outputFile, elem.second);
        encodingData.encodedStr.pop();
    }

    FileUtils::CloseFile(outputFile);
}

void CodecRLE::Decode(const char* inputPath, const char* outputPath) const
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = DecodeRLE(inputFile);
    FileUtils::AppendStr(outputFile, decodedStr);
    
    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// ==================================================================================
// Move-to-front

void CodecMTF::AlphabetShift(std::u32string& alphabet, const uint16_t& alphabetLength, const uint32_t& index) const
{
    char32_t temp = alphabet[0], temp2;
    for (uint32_t j = 1; j <= index; ++j) {
        temp2 = alphabet[j];
        alphabet[j] = temp;
        temp = temp2;
    }
    alphabet[0] = temp;
}

const uint32_t CodecMTF::GetIndex(const std::u32string& alphabet, const uint16_t alphabetLength, const char32_t c) const
{
    for (uint16_t i = 0; i < alphabetLength; ++i) {
        if (alphabet[i] == c) {
            return i;
        }
    }
    return 0; // assume that this will never happen
}

CodecMTF::dataMTF CodecMTF::GetDataMTF(const std::u32string& inputStr) const
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
    return dataMTF(alphabetLength, alphabet, strLength, codes);
}

std::string CodecMTF::DecodeMTF(FILE* inputFile) const
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

void CodecMTF::Encode(const char* inputPath, const char* outputPath) const
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    dataMTF encodingData = GetDataMTF(FileUtils::ReadContentToU32String(inputPath));
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

void CodecMTF::Decode(const char* inputPath, const char* outputPath) const
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = DecodeMTF(inputFile);
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// =================================================================================================
// Ariphmetical encoding

CodecAC::dataAC_local CodecAC::GetDataAC_local(const std::u32string& inputStr) const
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

    return dataAC_local(alphabetLength, alphabetLocal, frequencies, resultValue);
}

CodecAC::dataAC CodecAC::GetDataAC(const std::u32string& inputStr) const
{
    // maximum number of character in the string to make encoding 
    const uint8_t numChars = 13;
    std::queue<dataAC_local> queueLocalData; 

    uint64_t strLength = inputStr.size();
    // number of sequences to encode
    uint64_t numberOfFullSequences = (strLength % numChars == 0) ? (strLength / numChars) : (strLength / numChars);

    // encode every <numChars> characters
    uint64_t CountOfSeq = 0;
    while (CountOfSeq < numberOfFullSequences) {
        queueLocalData.push(GetDataAC_local(inputStr.substr(CountOfSeq * numChars, numChars)));
        ++CountOfSeq;
    }
    // handle the rest of the string
    if (strLength % numChars != 0) {
        queueLocalData.push(GetDataAC_local(inputStr.substr(numberOfFullSequences * numChars, strLength % numChars)));
    }

    return dataAC(strLength, queueLocalData);
}

std::string CodecAC::DecodeAC(FILE* inputFile) const
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

void CodecAC::Encode(const char* inputPath, const char* outputPath) const
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    dataAC encodingData = GetDataAC(FileUtils::ReadContentToU32String(inputPath));
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

void CodecAC::Decode(const char* inputPath, const char* outputPath) const
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = DecodeAC(inputFile);
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// ==================================================================================
// Huffman algorithm

uint8_t CodecHA::GetNumberFromBinaryString(const std::string& binaryString) const
{
    uint8_t result = 0;
    for (size_t i = 0; i < binaryString.size(); ++i) {
        if (binaryString[i] == '1') {
            result |= (1 << (binaryString.length() - i - 1));
        }
    }
    return result;
}

std::string CodecHA::GetBinaryStringFromNumber(const uint8_t& number, const uint8_t& codeLength) const
{
    std::string result(codeLength, '0');
    for (uint8_t i = 0; i < codeLength; ++i) {
        if (number & (1 << (codeLength - i - 1))) {
            result[i] = '1';
        }
    }
    return result;
} 

CodecHA::dataHA CodecHA::GetDataHA(const std::u32string& inputStr) const
{
    std::queue<dataHA_local> queueLocalData;

    const size_t strLengthToStart = 50;
    const size_t strLengthToAppend = 10;
    const size_t maxHuffmanCodeLength = 8;
    uint8_t maxHuffmanCodeLengthCounter;
    size_t stringPointer = 0;
    std::u32string localString;

    std::set<char32_t> alphabetSet;
    std::map<char32_t, size_t> charCountsMap;
    std::vector<std::pair<char32_t, double>> charFrequenciesVector;

    uint32_t temp;

    // get all the dataHA_local
    while (stringPointer != inputStr.size()) {
        // inicialization
        localString = inputStr.substr(stringPointer, strLengthToStart);
        alphabetSet = GetAlphabetSet(localString);
        charCountsMap = GetCharCountsMap(std::u32string(alphabetSet.begin(), alphabetSet.end()), alphabetSet.size(), localString);
        charFrequenciesVector = std::vector<std::pair<char32_t, double>>(charCountsMap.begin(), charCountsMap.end());
        for (size_t i = 0; i < charFrequenciesVector.size(); ++i) {
            charFrequenciesVector[i].second = static_cast<double>(charFrequenciesVector[i].second) / charCountsMap[charFrequenciesVector[i].first];
        }

        maxHuffmanCodeLengthCounter = 0;
        while (true) {
            // get maxHuffmanCodeLengthCounter
            HuffmanTree tree = BuildHuffmanTree(charFrequenciesVector, alphabetSet.size());
            maxHuffmanCodeLengthCounter = tree.GetHeight();
            if (maxHuffmanCodeLengthCounter > maxHuffmanCodeLength) {
                break;
            }

            // modify the data
            // expand localString
            localString += inputStr.substr(localString.size(), strLengthToAppend);
            // expand alphabetSet and charCountsMap
            temp = alphabetSet.size();
            for (size_t i = localString.size() - strLengthToAppend; i < localString.size(); ++i) {
                alphabetSet.insert(localString[i]);

                // if inderted character is new 
                if (alphabetSet.size() > temp) {
                    ++charCountsMap[localString[i]];
                    ++temp;
                }
            }
            // get charFrequenciesVector
            charFrequenciesVector = std::vector<std::pair<char32_t, double>>(charCountsMap.begin(), charCountsMap.end());
            for (size_t i = 0; i < charFrequenciesVector.size(); ++i) {
                charFrequenciesVector[i].second = static_cast<double>(charFrequenciesVector[i].second) / charCountsMap[charFrequenciesVector[i].first];
            }
            // sort vector by frequencies
            std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
            });
        }
        
        HuffmanTree tree = BuildHuffmanTree(charFrequenciesVector, alphabetSet.size());
        std::map<char32_t, std::string> huffmanCodesMap = GetHuffmanCodes(tree, alphabetSet.size());
        std::string encodedStr;
        for (size_t i = 0; i < inputStr.size(); ++i) {
            encodedStr += huffmanCodesMap[inputStr[i]];
        }
        queueLocalData.push(dataHA_local(alphabetSet.size(), std::u32string(alphabetSet.begin(), alphabetSet.end()), huffmanCodesMap, encodedStr));
        
        // move stringPointer
        stringPointer += localString.size();
    }
    
    return dataHA(queueLocalData);
}

std::u32string CodecHA::DecodeHA(FILE* inputFile) const
{
    uint64_t countOfLocalData = FileUtils::ReadValueBinary<uint64_t>(inputFile);
    std::u32string decodedStr;

    for (uint64_t i = 0; i < countOfLocalData; ++i) {
        uint32_t alphabetLength = FileUtils::ReadValueBinary<uint32_t>(inputFile);
        std::u32string alphabet = CodecUTF8::DecodeString32FromBinaryFile(inputFile, alphabetLength);

        uint16_t lengthsOfCodesSize = FileUtils::ReadValueBinary<uint16_t>(inputFile);
        std::string lengthsOfCodes = FileUtils::ReadSequenceOfDigitsBinary(inputFile, lengthsOfCodesSize);

        std::map<std::string, char32_t> inverseHuffmanCodesMap;
        for (uint32_t j = 0; j < alphabetLength; ++j) {
            inverseHuffmanCodesMap[GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), lengthsOfCodes[j])] = alphabet[i];
        }

        uint64_t encodedStrLength = FileUtils::ReadValueBinary<uint64_t>(inputFile);
        std::string encodedStr; encodedStr.reserve(encodedStrLength);
        for (uint64_t j = 0; j < encodedStrLength - encodedStrLength % 8; j += 8) {
            encodedStr += GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), 8);
        }
        if (encodedStrLength % 8 != 0) {
            encodedStr += GetBinaryStringFromNumber(FileUtils::ReadValueBinary<uint8_t>(inputFile), encodedStrLength % 8);
        }

        std::u32string decodedStrLocal;
        std::string currentCode; currentCode.reserve(8);
        for (size_t j = 0; j < encodedStr.size(); ++j) {
            currentCode.push_back(encodedStr[j]);
            if (inverseHuffmanCodesMap.find(currentCode) != inverseHuffmanCodesMap.end()) {
                decodedStrLocal.push_back(inverseHuffmanCodesMap[currentCode]);
                currentCode.clear();
            }
        }

        decodedStr += decodedStrLocal;
    }

    return decodedStr;
}

void CodecHA::Encode(const char* inputPath, const char* outputPath) const
{
    FILE* outputFile = FileUtils::OpenFileBinaryWrite(outputPath);

    dataHA encodingData = GetDataHA(FileUtils::ReadContentToU32String(inputPath));
    
    FileUtils::AppendValueBinary(outputFile, static_cast<uint64_t>(encodingData.queueLocalData.size()));
    while (!encodingData.queueLocalData.empty()) {
        dataHA_local dataLocal = encodingData.queueLocalData.front();

        FileUtils::AppendValueBinary(outputFile, dataLocal.alphabetLength);
        CodecUTF8::EncodeString32ToBinaryFile(outputFile, dataLocal.alphabet);

        std::map<char32_t, std::string> huffmanCodesMap = dataLocal.huffmanCodesMap;
        // write lengths of huffman codes effectively
        std::string lengthsOfCodes;
        for (size_t i = 0; i < dataLocal.alphabetLength; ++i) {
            //FileUtils::AppendValueBinary(outputFile, static_cast<uint8_t>(huffmanCodesMap[dataLocal.alphabet[i]].size()));
            lengthsOfCodes.push_back(huffmanCodesMap[dataLocal.alphabet[i]].size() + '0');
        }
        FileUtils::AppendValueBinary(outputFile, static_cast<uint16_t>(lengthsOfCodes.size()));
        FileUtils::AppendSequenceOfDigitsBinary(outputFile, lengthsOfCodes);
        // write huffman codes as a single string
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

void CodecHA::Decode(const char* inputPath, const char* outputPath) const
{
    FILE* inputFile = FileUtils::OpenFileBinaryRead(inputPath);
    std::ofstream outputFile = FileUtils::OpenFile<std::ofstream>(outputPath);

    std::string decodedStr = CodecUTF8::EncodeString32ToString(DecodeHA(inputFile));
    FileUtils::AppendStr(outputFile, decodedStr);

    FileUtils::CloseFile(outputFile);
    FileUtils::CloseFile(inputFile);
}

// ==================================================================================

/*
// ==================================================================================
// Burrows-Wheeler transform

void CodecBWT::EncodeBWT(const std::wstring& str, const std::string& outputPath) const
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

std::wstring CodecBWT::DecodeBWT(const std::string& inputPath) const
{
    // START READ METADATA
    MyFile file(inputPath, "r");

    uint64_t permutationsLength = file.ReadUint64Binary();
    // get sorted letters
    wchar_t* sortedLetters = new wchar_t[permutationsLength];
    for (uint64_t i = 0; i < permutationsLength; ++i) {
        sortedLetters[i] = file.ReadWideCharBinary();
    }
    uint64_t indexOfOriginal = file.ReadUint64Binary();

    // END READ METADATA

    // get permutations
    std::wstring* permutations = new std::wstring[permutationsLength];
    for (uint64_t i = 0; i < permutationsLength; ++i) {
        // add new column
        for (uint64_t j = 0; j < permutationsLength; ++j) {
            permutations[j].insert(0, 1, sortedLetters[j]);
        }

        // sort permutations
        //// stable sort is necessary to save order
        std::stable_sort(permutations, permutations + permutationsLength);
    }

    // get count of the same letters before the letter[indexOfOriginal]
    uint64_t count = 0;
    for (int64_t i = 0; i < indexOfOriginal; ++i) {
        if (sortedLetters[i] == sortedLetters[indexOfOriginal]) ++count;
    }

    delete[] sortedLetters;
    return permutations[indexOfOriginal];
}

void CodecBWT::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile file(inputPath, "r");
    std::wstring inputStr = file.ReadWideContent();
    EncodeBWT(inputStr, outputPath);
}

void CodecBWT::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    std::wstring result = DecodeBWT(inputPath);
    MyFile file(outputPath, "w");
    file.WriteWideContent(result);
}


// Huffman encoding

void CodecHUF::EncodeHUF(const std::wstring& str, const std::string& outputPath) const
{
    MyFile file(outputPath, "w");

    // initialize alphabet and char-frequency pairs sorted by char
    wchar_t* alphabet = Alphabet(str);
    int alphabetSize = wcslen(alphabet);
    std::pair<wchar_t, double>* charFrequencies = CharFrequencyPairs(alphabet, alphabetSize, str);
    // sort by frequencies
    std::sort(charFrequencies, charFrequencies + alphabetSize, [](const std::pair<wchar_t, double>& a, const std::pair<wchar_t, double>& b) {
        return a.second < b.second;
    });

    // build Huffman tree
    HuffmanTree tree = BuildHuffmanTree(charFrequencies, alphabetSize);
    // get Huffman codes
    std::vector<std::pair<wchar_t, std::string>> huffmanCodes = GetHuffmanCodes(tree, alphabetSize);
    // sort codes by chars
    std::sort(huffmanCodes.begin(), huffmanCodes.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    // make map of codes for fast access
    std::map<wchar_t, std::string> huffmanCodesMap(huffmanCodes.begin(), huffmanCodes.end());

    // write result
    // write alphabet size
    file.AppendInt32Binary(alphabetSize);
    // write alphabet
    for (int i = 0; i < alphabetSize; ++i) {
        file.AppendWideCharBinary(huffmanCodes[i].first);
    }
    // write huffman codes
    for (int i = 0; i < alphabetSize; ++i) {
        for (int j = 0; j < huffmanCodes[i].second.size(); ++j) {
            file.AppendCharBinary(huffmanCodes[i].second[j]);
        }
        file.AppendCharBinary(' ');
    }
    // write length of the string
    file.AppendInt64Binary(str.size());
    // write encoded string
    for (size_t i = 0; i < str.size(); ++i) {
        encodedStr += huffmanCodesMap[str[i]];
    }

    delete[] alphabet; delete[] charFrequencies;
}

std::wstring CodecHUF::DecodeHUF(const std::string& inputPath) const
{

    return L"";
}

void CodecHUF::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile file(inputPath, "r");
    std::wstring inputStr = file.ReadWideContent();
    EncodeHUF(inputStr, outputPath);
}

void CodecHUF::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    std::wstring result = DecodeHUF(inputPath);
    MyFile file(outputPath, "w");
    file.WriteWideContent(result);
}
 */
// END IMPLEMENTATION