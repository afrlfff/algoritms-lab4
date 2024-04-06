#pragma once

#include <string>
#include <set> // makes ordered set
#include <queue>
#include <algorithm> // sorting
#include <cstdint> // for int8_t, int16_t ...
#include <utility> // for std::pair

#include "FileUtils.h"
#include "TextTools.h"
#include "HuffmanTree.h"


class FileCodec
{
public:
    virtual void Encode(const char* inputPath, const char* outputPath) const = 0; 
    virtual void Decode(const char* inputPath, const char* outputPath) const = 0;
protected:
    FileCodec() = default;
    char WideCharToChar(const wchar_t& wc) const;
    std::string WstringToString(const std::wstring& wstr) const;
    int8_t Int16ToInt8(const int16_t& value) const;
};

// Run-length encoding
class CodecRLE : public FileCodec
{
public:
    CodecRLE() = default;
    void Encode(const char* inputPath, const char* outputPath) const override;
    void Decode(const char* inputPath, const char* outputPath) const override;
protected:
    template <typename stringType>
    struct dataRLE {
        size_t strSize;
        std::queue<std::pair<int8_t, stringType>> encodedStr;
        dataRLE(size_t _strSize, std::queue<std::pair<int8_t, stringType>> _encodedStr) : strSize(_strSize), encodedStr(_encodedStr) {}
    };

    // Encodes the input wstring with RLE algorithm and returns the data to write in the output file
    template <typename stringType, typename equalCharType>
    dataRLE<stringType> GetDataRLE(const stringType& inputStr) const;
    template <typename stringType>
    stringType DecodeRLE(FILE* inputFile) const;
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
        int16_t alphabetSize;
        std::wstring alphabet;
        int64_t strSize;
        std::vector<int16_t> codes;
        dataMTF(int16_t _alphabetSize, std::wstring _alphabet, int64_t _strSize, std::vector<int16_t> _codes) : alphabetSize(_alphabetSize), alphabet(_alphabet), strSize(_strSize), codes(_codes) {}
    };

    template <typename stringType, typename equalIntType>
    dataMTF GetDataMTF(const std::wstring& inputStr) const;
    std::string DecodeStringMTF(FILE* inputFile) const;
    std::wstring DecodeWstringMTF(FILE* inputFile) const;
};

/*
// Burrows-Wheeler transform
class CodecBWT : public FileCodec
{
    void EncodeBWT(const std::wstring& str, const std::string& outputPath) const;
    std::wstring DecodeBWT(const std::string& inputPath) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};

// Ariphmetical encoding
class CodecAFM : public FileCodec
{
    void EncodeAFM(const std::wstring& str, const std::string& outputPath) const;
    std::wstring DecodeAFM(const std::string& inputPath) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};

// Huffman encoding
class CodecHUF : public FileCodec
{
    void EncodeHUF(const std::wstring& str, const std::string& outputPath) const;
    std::wstring DecodeHUF(const std::string& inputPath) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};
 */

// START IMPLEMENTATION

// ==================================================================================
// File Codec

char FileCodec::WideCharToChar(const wchar_t& wc) const {
    return (char)(wc);
};

std::string FileCodec::WstringToString(const std::wstring& wstr) const {
    std::string newStr;
    for (size_t i = 0; i < wstr.size(); ++i) {
        newStr.push_back(WideCharToChar(wstr[i]));
    }
    return newStr;
};

int8_t FileCodec::Int16ToInt8(const int16_t& value) const {
    return (int8_t)(value);
}

// ==================================================================================
// Run-length encoding

template <typename stringType, typename equalCharType>
CodecRLE::dataRLE<stringType> CodecRLE::GetDataRLE(const stringType& inputStr) const
{
    std::queue<std::pair<int8_t, stringType>> encodedStr;

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    stringType uniqueSeq(1, inputStr[0]); // last sequence of unique characters
    bool flag = false; // show if previous character was part of sequence
    equalCharType prev = inputStr[0]; // previous character

    int8_t maxPossibleNumber = 127; // maximum possible value of int8_t

    // start RLE
    for (size_t i = 1; i < inputStr.size(); ++i)
    {
        if (inputStr[i] == prev) 
        {
            // record last sequence of unique symbols if it exists
            if (countUnique > 1) {
                uniqueSeq.pop_back(); // because "prev" was read as unique
                --countUnique; // because "prev" was read as unique

                countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
                encodedStr.push(std::make_pair(-countUnique, uniqueSeq));

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { ++countIdent; }
            
            countUnique = 0;
            uniqueSeq.clear();
            uniqueSeq.reserve(maxPossibleNumber);
        }
        else 
        {
            // record last sequence of identical symbols if it exists
            if (countIdent > 1) {
                if (countIdent >= maxPossibleNumber) {
                    for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                        encodedStr.push(std::make_pair(maxPossibleNumber, stringType(1, prev)));
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    encodedStr.push(std::make_pair(countIdent % maxPossibleNumber, stringType(1, prev)));
                }
                flag = true;
                countIdent = 1;
            } else if (countIdent == 0) {
                countIdent = 1;
            }

            if (flag) {
                countUnique = 1;
                uniqueSeq = inputStr[i];
                flag = false;
            } else {
                if (countUnique == 0) {
                    countUnique = 1;
                    uniqueSeq = prev;
                }

                countUnique++;
                uniqueSeq.push_back(inputStr[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == maxPossibleNumber) {
                encodedStr.push(std::make_pair(-countUnique, uniqueSeq));
                flag = true;
                countUnique = 0;
                uniqueSeq.clear();
                uniqueSeq.reserve(maxPossibleNumber);
            }
        }
        prev = inputStr[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= maxPossibleNumber) {
            for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                encodedStr.push(std::make_pair(maxPossibleNumber, stringType(1, prev)));
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            encodedStr.push(std::make_pair(countIdent % maxPossibleNumber, stringType(1, prev)));
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        encodedStr.push(std::make_pair(-countUnique, uniqueSeq));
    }

    return dataRLE(inputStr.size(), encodedStr);
}

template <typename stringType>
stringType CodecRLE::DecodeRLE(FILE* inputFile) const
{
    stringType decodedStr;

    int64_t strSize = FileUtils::ReadInt64Binary(inputFile);
    int64_t counter = 0;
    int8_t number;
    while (counter < strSize)
    {
        number = FileUtils::ReadInt8Binary(inputFile);

        // if starts with negative number
        // (sequence of unqiue symbols)
        if (number < 0)
        {
            for (int8_t i = 0; i < (-number); ++i) {
                if (std::is_same<stringType, std::wstring>::value) {
                    decodedStr.push_back(FileUtils::ReadWideCharBinary(inputFile));
                } else {
                    decodedStr.push_back(FileUtils::ReadCharBinary(inputFile));
                }
                ++counter;
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            if (std::is_same<stringType, std::wstring>::value) {
                wchar_t c = FileUtils::ReadWideCharBinary(inputFile);
                for (int8_t i = 0; i < number; ++i) {
                    decodedStr.push_back(c);
                    ++counter;
                }
            } else {
                char c = FileUtils::ReadCharBinary(inputFile);
                for (int8_t i = 0; i < number; ++i) {
                    decodedStr.push_back(c);
                    ++counter;
                }
            }
        }
    }

    return decodedStr;
}

void CodecRLE::Encode(const char* inputPath, const char* outputPath) const
{
    FILE* outputFile = FileUtils::OpenWriteBinary(outputPath);

    if (FileUtils::ContainsWideChars(inputPath)) {
        dataRLE encodingData = GetDataRLE<std::wstring, wchar_t>(FileUtils::ReadWideContent(inputPath));
        FileUtils::AppendCharBinary(outputFile, 'w');
        FileUtils::AppendInt64Binary(outputFile, encodingData.strSize);
        while (!encodingData.encodedStr.empty()) {
            auto elem = encodingData.encodedStr.front();
            FileUtils::AppendInt8Binary(outputFile, elem.first);
            FileUtils::AppendWideStrBinary(outputFile, elem.second);
            encodingData.encodedStr.pop();
        }
    } else {
        dataRLE encodingData = GetDataRLE<std::string, char>(FileUtils::ReadContent(inputPath));
        FileUtils::AppendCharBinary(outputFile, 'c');
        FileUtils::AppendInt64Binary(outputFile, encodingData.strSize);
        while (!encodingData.encodedStr.empty()) {
            auto elem = encodingData.encodedStr.front();
            FileUtils::AppendInt8Binary(outputFile, elem.first);
            FileUtils::AppendStrBinary(outputFile, elem.second);
            encodingData.encodedStr.pop();
        }
    }
    FileUtils::CloseFile(outputFile);
}

void CodecRLE::Decode(const char* inputPath, const char* outputPath) const
{
    FILE* inputFile = FileUtils::OpenReadBinary(inputPath);
    char flag = FileUtils::ReadCharBinary(inputFile);

    if (flag == 'w') {
        std::wofstream outputFile = FileUtils::OpenWriteWide(outputPath);
        std::wstring decodedStr = DecodeRLE<std::wstring>(inputFile);
        FileUtils::AppendWideStr(outputFile, decodedStr);
        FileUtils::CloseFile(outputFile);
    } else { // flag == 'c'
        std::ofstream outputFile = FileUtils::OpenWrite(outputPath);
        std::string decodedStr = DecodeRLE<std::string>(inputFile);
        FileUtils::AppendStr(outputFile, decodedStr);
        FileUtils::CloseFile(outputFile);
    }
    FileUtils::CloseFile(inputFile);
}

// ==================================================================================
// Move-to-front
/* 
template <typename stringType, typename equalIntType>
CodecMTF::dataMTF CodecMTF::GetDataMTF(const std::wstring& inputStr) const
{
    std::wstring alphabet = Alphabet(inputStr);
    size_t alphabetLength = alphabet.size();

    if (alphabetLength <= 256) {
        EncodeMTF8(alphabet, alphabetLength, str, file);
    } else {
        EncodeMTF16(alphabet, alphabetLength, str, file);
    }

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

}

void CodecMTF::EncodeMTF(const std::wstring& str, const std::string& outputPath) const
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

std::wstring CodecMTF::DecodeMTF(const std::string& inputPath) const
{
    auto DecodeMTF8 = [](wchar_t*& alphabet, const int64_t strLength, MyFile& f) {
        std::wstring decodedStr = L"";
        uint8_t index;
        wchar_t temp, temp2;
        for (int64_t i = 0; i < strLength; ++i) {
            index = f.ReadUint8Binary();
            decodedStr.push_back(alphabet[index]);

            // shift right
            temp = alphabet[0];
            for (size_t j = 1; j <= index; ++j) {
                temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
        return decodedStr;
    };
    auto DecodeMTF16 = [](wchar_t*& alphabet, const int64_t strLength, MyFile& f) {
        std::wstring decodedStr = L"";
        for (int64_t i = 0; i < strLength; ++i) {
            int16_t index = f.ReadUint16Binary();
            decodedStr.push_back(alphabet[index]);

            // shift right
            wchar_t temp = alphabet[0];
            for (size_t j = 1; j <= index; ++j) {
                wchar_t temp2 = alphabet[j];
                alphabet[j] = temp;
                temp = temp2;
            }
            alphabet[0] = temp;
        }
        return decodedStr;
    };

    MyFile file(inputPath, "r");

    // read meta data
    uint16_t alphabetLength = file.ReadUint16Binary();
    wchar_t* alphabet = new wchar_t[alphabetLength + 1];
    for (size_t i = 0; i < alphabetLength; ++i) {
        alphabet[i] = file.ReadWideCharBinary();
    }
    alphabet[alphabetLength] = L'\0';
    int64_t strLength = file.ReadInt64Binary();

    // decode
    std::wstring decodedStr;
    if (alphabetLength <= 256) {
        decodedStr = DecodeMTF8(alphabet, strLength, file);
    } else {
        decodedStr = DecodeMTF16(alphabet, strLength, file);
    }

    return decodedStr;
}

void CodecMTF::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile file(inputPath, "r");
    std::wstring inputStr = file.ReadWideContent();
    EncodeMTF(inputStr, outputPath);
}

void CodecMTF::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    std::wstring result = DecodeMTF(inputPath);
    MyFile file(outputPath, "w");
    file.WriteWideContent(result);
}
 */
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

// Ariphmetical encoding

void CodecAFM::EncodeAFM(const std::wstring& str, const std::string& outputPath) const
{
    auto encode = [](const std::wstring& str, MyFile& file) {
        // initialize sorted alphabet and sorted frequencies
        wchar_t* alphabet = Alphabet(str);
        int8_t size = wcslen(alphabet);
        std::pair<wchar_t, double>* charFrequencies = CharFrequencyPairs(alphabet, size, str);
        std::sort(charFrequencies, charFrequencies + size);

        // leave in frequencies only 2 characters after the decimal point
        // (for correct decoding)
        for (int8_t i = 0; i < size; ++i) {
            charFrequencies[i].second = ((int8_t)(charFrequencies[i].second * 100)) / 100.0;
        }

        // inicialize segments
        //// (array of bounds points from 0 to 1)
        double* segments = new double[size + 1]{ 0 };
        for (int i = 1; i < size; ++i) {
            segments[i] = charFrequencies[i - 1].second + segments[i - 1];
        }
        segments[size] = 1;

        // encode (get final left and right bounds)
        double leftBound = 0, rightBound = 1, distance;
        for (wchar_t c : str) {
            int8_t index = GetIndexInSorted(charFrequencies, size, c);
            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        // write size of alphabet
        file.AppendInt8Binary(size);

        // write alphabet
        for (int8_t i = 0; i < size; ++i) {
            file.AppendWideCharBinary(charFrequencies[i].first);
        }

        // write frequencies
        for (int8_t i = 0; i < size; ++i){
            file.AppendInt8Binary((int8_t)((charFrequencies[i].second) * 100));
        }

        double resultValue = (rightBound + leftBound) / 2;
        // leave only 9 digits after the decimal point
        //// cause int32_t value can store any number with 9 digits 
        file.AppendInt32Binary((int32_t)(resultValue * 1000000000));

        delete[] alphabet; delete[] charFrequencies; delete[] segments;
    };

    MyFile file(outputPath, "w");
    int64_t size = str.size();

    // write count of sequences
    int64_t countOfSequences = (size % 9 == 0) ? (size / 9) : (size / 9 + 1);
    file.AppendInt64Binary(countOfSequences);

    // encode every 9 chars
    int64_t i = 0;
    while (i + 9 <= size) {
        encode(str.substr(i, 9), file);
        i += 9;
    }
    // handle the rest of the string
    if (i != size) {
        encode(str.substr(i, size - i), file);
        file.AppendInt8Binary(size - i); // fix last count of characters
                                         //(cause it can be lower than 9)
    } else {
        file.AppendInt8Binary(9); // fix last count of characters
    }
}

std::wstring CodecAFM::DecodeAFM(const std::string& inputPath) const
{
    auto decode = [](const wchar_t alhpabet[10], int8_t alphabetSize, const double frequencies[9], double resultValue, int8_t countOfIterations) {
        // inicialize segments
        //// (array of bound points from 0 to 1)
        double* segments = new double[alphabetSize + 1]{ 0 };
        for (int8_t i = 1; i < alphabetSize; ++i) {
            segments[i] = frequencies[i - 1] + segments[i - 1];
        }
        segments[alphabetSize] = 1;

        // decode
        std::wstring result;
        double leftBound = 0, rightBound = 1, distance;
        int index;
        for (int8_t i = 0; i < countOfIterations; ++i) {
            // find index of segment contains resultValue
            for (int8_t j = 0; j < alphabetSize; ++j) {
                if (resultValue >= (leftBound + segments[j] * (rightBound - leftBound)) && 
                    resultValue < (leftBound + segments[j + 1] * (rightBound - leftBound))) {
                    index = j;
                    break;
                }
            }
            result.push_back(alhpabet[index]);

            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        delete[] segments;
        return result;
    };

    std::wstring result = L""; 
    MyFile file(inputPath, "r");

    // get count of sequences
    int64_t countOfSequences = file.ReadInt64Binary();

    // get sequences and merge results
    int8_t alphabetSize;
    wchar_t alhpabet[9 + 1]; // can't contain more than 9 characters
    double frequencies[9]; // can't contain more than 9 characters
    double resultValue;
    int8_t lastCount = 9;
    for (int64_t i = 0; i < countOfSequences; ++i) {
        // read size of alphabet
        alphabetSize = file.ReadInt8Binary();

        // read alhpabet
        for (int8_t j = 0; j < alphabetSize; ++j) {
            alhpabet[j] = file.ReadWideCharBinary();
        }
        alhpabet[alphabetSize] = L'\0';

        // read frequencies
        for (int8_t j = 0; j < alphabetSize; ++j) {
            frequencies[j] = file.ReadInt8Binary() / 100.0;
        }

        // read result value
        resultValue = file.ReadInt32Binary() / 1000000000.0;

        // read last count if it exists
        if (i == countOfSequences - 1) { // if last iteration
            lastCount = file.ReadInt8Binary();
        }

        result += decode(alhpabet, alphabetSize, frequencies, resultValue, lastCount);
    }

    return result;
}

void CodecAFM::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile file(inputPath, "r");
    std::wstring inputStr = file.ReadWideContent();
    EncodeAFM(inputStr, outputPath);
}

void CodecAFM::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    std::wstring result = DecodeAFM(inputPath);
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