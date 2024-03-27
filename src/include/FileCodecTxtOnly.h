#pragma once

#include <string>
#include <set> // makes ordered set
#include <algorithm> // sorting
#include <cstdint> // for int8_t, int16_t ...

#include "MyFile.h"
#include "TextTools.h"

/**
 * This class made only to test encode and decode functions
 * to clearly see how it works in .txt files insted of using .bin files
*/
class FileCodecTxtOnly
{
public:
    virtual void Encode(const std::string& inputPath, const std::string& outputPath) const = 0;
    virtual void Decode(const std::string& inputPath, const std::string& outputPath) const = 0;
};

// Run-length encoding
/**
 * This class made only to test encode and decode functions
 * to clearly see how it works in .txt files insted of using .bin files
*/
class CodecRLETxtOnly : public FileCodecTxtOnly
{
    std::wstring EncodeRLE(const std::wstring& str) const;
    std::wstring DecodeRLE(const std::wstring& str) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};

// Move-to-front
/**
 * This class made only to test encode and decode functions
 * to clearly see how it works in .txt files insted of using .bin files
*/
class CodecMTFTxtOnly : public FileCodecTxtOnly
{
    std::wstring EncodeMTF(const std::wstring& str) const;
    std::wstring DecodeMTF(const std::wstring& str) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};

// Burrows-Wheeler transform
/**
 * This class made only to test encode and decode functions
 * to clearly see how it works in .txt files insted of using .bin files
*/
class CodecBWTTxtOnly : public FileCodecTxtOnly
{
    std::wstring EncodeBWT(const std::wstring& str) const;
    std::wstring DecodeBWT(const std::wstring& str) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};

// Ariphmetical encoding
/**
 * This class made only to test encode and decode functions
 * to clearly see how it works in .txt files insted of using .bin files
*/
class CodecAFMTxtOnly : public FileCodecTxtOnly
{
    std::wstring EncodeAFM(const std::wstring& str) const;
    std::wstring DecodeAFM(const std::wstring& str) const;
public:
    void Encode(const std::string& inputPath, const std::string& outputPath) const override;
    void Decode(const std::string& inputPath, const std::string& outputPath) const override;
};


// START IMPLEMENTATION

// Run-length encoding

std::wstring CodecRLETxtOnly::EncodeRLE(const std::wstring& str) const
{
    std::wstring newStr = L"";

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::wstring uniqueSeq(1, str[0]); // last sequence of unique characters

    // show if previous character was part of sequence
    bool flag = false;

    wchar_t prev = str[0]; // previous character

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
                newStr += (std::to_wstring(-1 * countUnique) + uniqueSeq);

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
                if (countIdent >= 9) {
                    for (size_t i = 0; i < (countIdent / 9); i++) {
                        newStr.push_back(L'9');
                        newStr.push_back(prev);
                    }
                }
                if (countIdent % 9 != 0) {
                    newStr.push_back((L'0' + (countIdent % 9)));
                    newStr.push_back(prev);
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

                ++countUnique;
                uniqueSeq.push_back(str[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == 9) {
                newStr.append(std::to_wstring(-1 * countUnique) + uniqueSeq);
                flag = true;
                countUnique = 0;
                uniqueSeq = L"";
            }
        }
        prev = str[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= 9) {
            for (size_t i = 0; i < (countIdent / 9); ++i) {
                newStr.push_back(L'9');
                newStr.push_back(prev);
            }
        }
        if (countIdent % 9 != 0) {
            newStr.push_back((L'0' + (countIdent % 9)));
            newStr.push_back(prev);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        newStr.append(std::to_wstring(-1 * countUnique) + uniqueSeq);
    }

    return newStr;
}

std::wstring CodecRLETxtOnly::DecodeRLE(const std::wstring& str) const
{
    std::wstring newStr = L"";

    size_t i = 0;
    int8_t count;

    while (i < str.length())
    {
        // if starts with negative number
        // (sequence of unqiue symbols)
        if (str[i] == '-')
        {
            count = str[i + 1] - L'0'; // convert char to int
            i += 2;
            for (size_t j = i; j < (i + count); j++) {
                newStr.push_back(str[j]);
            }
            i += count;
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            count = str[i] - L'0';
            newStr.append(count, str[i + 1]);
            i += 2;
        }
    }

    return newStr;
}

void CodecRLETxtOnly::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring originalStr = inputFile.ReadWideContent();
    std::wstring encodedStr = EncodeRLE(originalStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(encodedStr);
}

void CodecRLETxtOnly::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring encodedStr = inputFile.ReadWideContent();
    std::wstring decodedStr = DecodeRLE(encodedStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(decodedStr);
}

// Move-to-front

std::wstring CodecMTFTxtOnly::EncodeMTF(const std::wstring& str) const
{
    std::wstring encodedStr;
    wchar_t* alphabet = Alphabet(str);
    int alphabetLength = wcslen(alphabet);

    // write length of alphabet
    encodedStr.append(std::to_wstring(alphabetLength) + L'\n');
    // write alphabet
    for (int i = 0; i < alphabetLength; ++i) {
        encodedStr.push_back(alphabet[i]);
    }
    encodedStr += '\n';

    // move-to-front
    for (size_t i = 0; i < str.size(); ++i) {
        int index = GetIndex(alphabet, alphabetLength, str[i]);
        encodedStr += std::to_wstring(index) + L' ';

        // shift to the right
        wchar_t temp = alphabet[0];
        for (size_t j = 1; j <= index; ++j) {
            wchar_t temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;
    }

    return encodedStr;
}

std::wstring CodecMTFTxtOnly::DecodeMTF(const std::wstring& str) const
{
    wchar_t c;
    size_t i = 0;
    int alphabetLength;
    wchar_t* alphabet;

    // read alphabet length
    wchar_t buffer[10];
    while (str[i] != L'\n') {
        buffer[i++] = str[i];
    }
    alphabetLength = std::stoi(buffer);

    // read alphabet
    ++i;
    alphabet = new wchar_t[alphabetLength + 1];
    for (size_t j = 0; j < alphabetLength; ++j) {
        alphabet[j] = str[i++];
    }
    alphabet[alphabetLength] = L'\0';

    // decode
    ++i;
    std::wstring decodedStr = L"";
    while (i < str.size()) {
        std::wstring numberStr = L"";
        while (str[i] != L' ') {
            numberStr.push_back(str[i++]);
        }
        size_t index = std::stoi(numberStr);
        decodedStr.push_back(alphabet[index]);

        // shift right
        wchar_t temp = alphabet[0];
        for (size_t j = 1; j <= index; ++j) {
            wchar_t temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;

        ++i;
    }

    delete[] alphabet;
    return decodedStr;
}

void CodecMTFTxtOnly::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring originalStr = inputFile.ReadWideContent();
    std::wstring encodedStr = EncodeMTF(originalStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(encodedStr);
}

void CodecMTFTxtOnly::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring encodedStr = inputFile.ReadWideContent();
    std::wstring decodedStr = DecodeMTF(encodedStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(decodedStr);
}

// Burrows-Wheeler transform

std::wstring CodecBWTTxtOnly::EncodeBWT(const std::wstring& str) const
{
    size_t index = -1, permutationsLength = 0;

    // get index and length of permutations
    for (size_t i = 0; i < str.size(); ++i) {
        if (index != -1) ++permutationsLength;
        if (str[i] == L'\n') {
            index  = std::stoi(str.substr(0, i));
        }
    }

    // get permutations
    std::wstring sortedLetters = str.substr(str.size() - permutationsLength, permutationsLength);
    std::wstring* permutations = new std::wstring[permutationsLength];
    for (size_t i = 0; i < permutationsLength; ++i) {
        // add new column
        for (size_t j = 0; j < permutationsLength; ++j) {
            permutations[j].insert(0, 1, sortedLetters[j]);
        }

        // sort permutations
        std::stable_sort(permutations, permutations + permutationsLength);
    }

    // get count of the same letters before the letter[index]
    size_t count = 0;
    for (size_t i = 0; i < index; i++) {
        if (sortedLetters[i] == sortedLetters[index]) count++;
    }

    return permutations[index];
}

std::wstring CodecBWTTxtOnly::DecodeBWT(const std::wstring& str) const
{
    size_t index = -1, permutationsLength = 0;

    // get index and length of permutations
    for (size_t i = 0; i < str.size(); ++i) {
        if (index != -1) ++permutationsLength;
        if (str[i] == L'\n') {
            index  = std::stoi(str.substr(0, i));
        }
    }

    // get permutations
    std::wstring sortedLetters = str.substr(str.size() - permutationsLength, permutationsLength);
    std::wstring* permutations = new std::wstring[permutationsLength];
    for (size_t i = 0; i < permutationsLength; ++i) {
        // add new column
        for (size_t j = 0; j < permutationsLength; ++j) {
            permutations[j].insert(0, 1, sortedLetters[j]);
        }

        // sort permutations
        std::stable_sort(permutations, permutations + permutationsLength);
    }

    // get count of the same letters before the letter[index]
    size_t count = 0;
    for (size_t i = 0; i < index; i++) {
        if (sortedLetters[i] == sortedLetters[index]) count++;
    }

    return permutations[index];
}

void CodecBWTTxtOnly::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring originalStr = inputFile.ReadWideContent();
    std::wstring encodedStr = EncodeBWT(originalStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(encodedStr);
}

void CodecBWTTxtOnly::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring encodedStr = inputFile.ReadWideContent();
    std::wstring decodedStr = DecodeBWT(encodedStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(decodedStr);
}

// Ariphmetical encoding

std::wstring CodecAFMTxtOnly::EncodeAFM(const std::wstring& str) const
{
    auto encode = [](const std::wstring& str) {
        // initialize sorted alphabet and sorted frequencies
        wchar_t* alphabet = Alphabet(str);
        int size = wcslen(alphabet);
        std::pair<wchar_t, double>* frequencies = Frequencies(alphabet, size, str);
        std::sort(frequencies, frequencies + size);

        // leave in frequencies only 2 characters after the decimal point
        // (for correct decoding)
        for (int i = 0; i < size; ++i) {
            frequencies[i].second = ((int)(frequencies[i].second * 100)) / 100.0;
        }

        // inicialize segments
        //// (array of bounds points from 0 to 1)
        double* segments = new double[size + 1]{ 0 };
        for (int i = 1; i < size; ++i) {
            segments[i] = frequencies[i - 1].second + segments[i - 1];
        }
        segments[size] = 1;

        // encode
        double leftBound = 0, rightBound = 1, distance;
        for (wchar_t c : str) {
            int index = GetIndexInSorted(frequencies, size, c);
            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        // make result
        std::wstring result = std::to_wstring(size) + L'\n';
        for (int i = 0; i < size; ++i) {
            result.push_back(alphabet[i]);
        }
        result.push_back('\n'); 
        for (int i = 0; i < size; ++i){
            result += std::to_wstring((int8_t)((frequencies[i].second) * 100)) + L' ';
        }
        result.push_back('\n');

        double resultValue = (rightBound + leftBound) / 2;
        // leave only 9 digits after the decimal point
        //// cause int value can store any number with 9 digits 
        result += std::to_wstring((int)(resultValue * 1000000000));

        delete[] alphabet; delete[] frequencies; delete[] segments;
        return result;
    };

    std::wstring result = L"";
    size_t size = str.size();

    // write count of sequences
    size_t countOfSequences = (size % 9 == 0) ? (size / 9) : (size / 9 + 1);
    result += std::to_wstring(countOfSequences) + L'\n';

    // encode every 9 chars
    size_t i = 0;
    while (i + 9 <= size) {
        result += encode(str.substr(i, 9)) + L'\n';
        i += 9;
    }
    // handle the rest of the string
    if (i != size) {
        result += encode(str.substr(i, size - i)) + L'\n';
        result += std::to_wstring(size - i) + L'\n'; // fix last count of characters (cause it lower that 9)
    } else {
        result += std::to_wstring(9) + L'\n';
    }

    return result;
}

std::wstring CodecAFMTxtOnly::DecodeAFM(const std::wstring& str) const
{
    auto decode = [](const wchar_t alhpabet[10], int alphabetSize, const double frequencies[9], double resultValue, int countOfIterations) {
        // inicialize segments
        //// (array of bound points from 0 to 1)
        double* segments = new double[alphabetSize + 1]{ 0 };
        for (int i = 1; i < alphabetSize; ++i) {
            segments[i] = frequencies[i - 1] + segments[i - 1];
        }
        segments[alphabetSize] = 1;

        // decode
        std::wstring result;
        double leftBound = 0, rightBound = 1, distance;
        int index;
        for (int i = 0; i < countOfIterations; ++i) {
            // find index of segment that contains resultValue
            for (int j = 0; j < alphabetSize; ++j) {
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

    size_t i = 0, // index in str
           bufferIndex; // index in buffer
    wchar_t buffer[20];
    std::wstring result = L""; 

    // get count of sequences
    size_t countOfSequences;
    bufferIndex = 0;
    while (str[i] != L'\n') {
        buffer[bufferIndex++] = str[i];
        ++i;
    }
    buffer[bufferIndex] = L'\0';
    ++i; // to skip '\n'
    countOfSequences = std::stoi(buffer);

    // get sequences and merge results
    int alphabetSize;
    wchar_t alhpabet[9 + 1]; // can't contain more that 9 characters
    double frequencies[9];
    double resultValue;
    int lastCount = 9;
    for (size_t j = 0; j < countOfSequences; ++j) {
        // read size of alphabet
        bufferIndex = 0;
        while (str[i] != L'\n') {
            buffer[bufferIndex++] = str[i];
            ++i;
        }
        buffer[bufferIndex] = L'\0';
        ++i; // i++ to skip '\n'
        alphabetSize = std::stoi(buffer);

        // read alhpabet
        for (int k = 0; k < alphabetSize; ++k) {
            alhpabet[k] = str[i];
            ++i;
        }
        alhpabet[alphabetSize] = L'\0';
        ++i; // to skip '\n'

        // read frequencies
        for (int k = 0; k < alphabetSize; ++k) {
            bufferIndex = 0;
            while (str[i] != L' ' && str[i] != L'\n') {
                buffer[bufferIndex++] = str[i];
                ++i;
            }

            buffer[bufferIndex++] = L'\0';
            frequencies[k] = std::stoi(buffer) / 100.0;
            ++i; // to skip ' '
        }

        // read result value
        ++i; // to skip '\n'
        bufferIndex = 0;
        while (str[i] != L'\n') {
            buffer[bufferIndex++] = str[i];
            ++i;
        }
        buffer[bufferIndex] = L'\0';
        ++i; // to skip '\n'
        resultValue = std::stoi(buffer) / 1000000000.0;

        // read last count if it exists
        if (j == countOfSequences - 1) { // if last iteration
            bufferIndex = 0;
            while (str[i] != L'\n') {
                buffer[bufferIndex++] = str[i];
                ++i;
            }
            buffer[bufferIndex] = L'\0';
            lastCount = std::stoi(buffer);
            ++i; // to skip '\n'
        }

        result += decode(alhpabet, alphabetSize, frequencies, resultValue, lastCount);
    }

    return result;
}

void CodecAFMTxtOnly::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring originalStr = inputFile.ReadWideContent();
    std::wstring encodedStr = EncodeAFM(originalStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(encodedStr);
}

void CodecAFMTxtOnly::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile inputFile(inputPath, "r");
    std::wstring encodedStr = inputFile.ReadWideContent();
    std::wstring decodedStr = DecodeAFM(encodedStr);

    MyFile outputFile(outputPath, "w");
    outputFile.WriteWideContent(decodedStr);
}

// END IMPLEMENTATION