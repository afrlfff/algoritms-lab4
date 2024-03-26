#pragma once

#include <string>
#include <set> // makes ordered set
#include <algorithm> // sorting
#include <cstdint> // for int8_t, int16_t ...

#include "MyFile.h"
#include "TextTools.h"

class FileCodec
{
    std::string key;

    void EncodeRLE(const std::wstring& str, const std::string& outputPath) const; // Run-length encoding
    void EncodeMTF(const std::wstring& str, const std::string& outputPath) const; // Move-to-front encoding
    void EncodeBWT(const std::wstring& str, const std::string& outputPath) const; // Burrows-Wheeler transform
    void EncodeAFM(const std::wstring& str, const std::string& outputPath) const; // Ariphmetical encoding
    std::wstring DecodeRLE(const std::string& inputPath) const; // Run-length encoding
    std::wstring DecodeMTF(const std::string& inputPath) const; // Move-to-front
    std::wstring DecodeBWT(const std::string& inputPath) const; // Burrows-Wheeler transform
    std::wstring DecodeAFM(const std::string& inputPath) const; // Ariphmetical encoding
public:
    FileCodec(const char* key);

    std::string getKey() const { return key; };
    void Encode(const std::string& inputPath, const std::string& outputPath) const;
    void Decode(const std::string& inputPath, const std::string& outputPath) const;
};

// START IMPLEMENTATION

// ==============================================================================
// PUBLIC FUNCTIONS
// ==============================================================================

/**
 * Constructor for the MyFile class.
 *
 * @param key key which indicates the type encoding algorithm 
 *            (possible values: "RLE", "MTF", "BWT", "AFM").
 */
FileCodec::FileCodec(const char* key)
{
    if (key != "RLE" && key != "MTF" && key != "BWT" && key != "AFM") {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        exit(1);
    }

    int i = 0;
    while (key[i] != '\0') {
        this->key.push_back(key[i++]);
    }
}

void FileCodec::Encode(const std::string& inputPath, const std::string& outputPath) const
{
    MyFile file(inputPath, "r");
    std::wstring inputStr = file.ReadWideContent();

    if (key == "RLE") {
        EncodeRLE(inputStr, outputPath);
    } else if (key == "MTF") {
        return EncodeMTF(inputStr, outputPath);
    } else if (key == "BWT") {
        return EncodeBWT(inputStr, outputPath);
    } else if (key == "AFM") {
        return EncodeAFM(inputStr, outputPath);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
    }
}

void FileCodec::Decode(const std::string& inputPath, const std::string& outputPath) const
{
    std::wstring result = L"";

    if (key == "RLE") {
        result = DecodeRLE(inputPath);
    } else if (key == "MTF") {
        result = DecodeMTF(inputPath);
    } else if (key == "BWT") {
        result = DecodeBWT(inputPath);
    } else if (key == "AFM") {
        result = DecodeAFM(inputPath);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return;
    }

    MyFile file(outputPath, "w");
    file.WriteWideContent(result);
}

// ==============================================================================
// PRIVATE FUNCTIONS
// ==============================================================================

void FileCodec::EncodeRLE(const std::wstring& str, const std::string& outputPath) const
{
    MyFile file(outputPath, "w");

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::wstring uniqueSeq(1, str[0]); // last sequence of unique characters
    bool flag = false; // show if previous character was part of sequence
    wchar_t prev = str[0]; // previous character

    int8_t maxPossibleNumber = 127; // maximum possible value of int8_t

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
                file.AppendInt8Binary(-countUnique);
                file.AppendWideStrBinary(uniqueSeq);

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
                if (countIdent >= maxPossibleNumber) {
                    for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                        file.AppendInt8Binary(maxPossibleNumber);
                        file.AppendWideCharBinary(prev);
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    file.AppendInt8Binary(countIdent % maxPossibleNumber);
                    file.AppendWideCharBinary(prev);
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

                countUnique++;
                uniqueSeq.push_back(str[i]);
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == maxPossibleNumber) {
                file.AppendInt8Binary(-countUnique);
                file.AppendWideStrBinary(uniqueSeq);
                flag = true;
                countUnique = 0;
                uniqueSeq = L"";
            }
        }
        prev = str[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= maxPossibleNumber) {
            for (int i = 0; i < (countIdent / maxPossibleNumber); ++i) {
                file.AppendInt8Binary(maxPossibleNumber);
                file.AppendWideCharBinary(prev);
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            file.AppendInt8Binary(countIdent % maxPossibleNumber);
            file.AppendWideCharBinary(prev);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        file.AppendInt8Binary(-countUnique);
        file.AppendWideStrBinary(uniqueSeq);
    }
}

std::wstring FileCodec::DecodeRLE(const std::string& inputPath) const
{
    MyFile file(inputPath, "r");
    std::wstring newStr = L"";

    int8_t count;

    while (true)
    {
        count = file.ReadInt8Binary();

        // if count == 0 - end of file
        if (count == 0) break;

        // if starts with negative number
        // (sequence of unqiue symbols)
        if (count < 0)
        {
            for (int8_t i = 0; i < (-count); ++i) {
                newStr.push_back(file.ReadWideCharBinary());
            }
        }
        // if starts with positive number
        // (sequence of identical symbols)
        else
        {
            wchar_t c = file.ReadWideCharBinary();
            for (int8_t i = 0; i < count; ++i) {
                newStr.push_back(c);
            }
        }
    }

    return newStr;
}

// ==============================================================================

void FileCodec::EncodeMTF(const std::wstring& str, const std::string& outputPath) const
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

std::wstring FileCodec::DecodeMTF(const std::string& inputPath) const
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

// ==============================================================================

void FileCodec::EncodeBWT(const std::wstring& str, const std::string& outputPath) const
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

std::wstring FileCodec::DecodeBWT(const std::string& inputPath) const
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

// ==============================================================================

void FileCodec::EncodeAFM(const std::wstring& str, const std::string& outputPath) const
{
    auto encode = [](const std::wstring& str, MyFile& file) {
        // initialize sorted alphabet and sorted frequencies
        wchar_t* alphabet = Alphabet(str);
        int8_t size = wcslen(alphabet);
        std::pair<wchar_t, double>* frequencies = Frequencies(alphabet, size, str);
        std::sort(frequencies, frequencies + size);

        // leave in frequencies only 2 characters after the decimal point
        // (for correct decoding)
        for (int8_t i = 0; i < size; ++i) {
            frequencies[i].second = ((int8_t)(frequencies[i].second * 100)) / 100.0;
        }

        // inicialize segments
        //// (array of bounds points from 0 to 1)
        double* segments = new double[size + 1]{ 0 };
        for (int i = 1; i < size; ++i) {
            segments[i] = frequencies[i - 1].second + segments[i - 1];
        }
        segments[size] = 1;

        // encode (get final left and right bounds)
        double leftBound = 0, rightBound = 1, distance;
        for (wchar_t c : str) {
            int8_t index = GetIndexInSorted(frequencies, size, c);
            distance = rightBound - leftBound;
            rightBound = leftBound + segments[index + 1] * distance;
            leftBound = leftBound + segments[index] * distance;
        }

        // write size of alphabet
        file.AppendInt8Binary(size);

        // write alphabet
        for (int8_t i = 0; i < size; ++i) {
            file.AppendWideCharBinary(alphabet[i]);
        }

        // write frequencies
        for (int8_t i = 0; i < size; ++i){
            file.AppendInt8Binary((int8_t)((frequencies[i].second) * 100));
        }

        double resultValue = (rightBound + leftBound) / 2;
        // leave only 9 digits after the decimal point
        //// cause int32_t value can store any number with 9 digits 
        file.AppendInt32Binary((int32_t)(resultValue * 1000000000));

        delete[] alphabet; delete[] frequencies; delete[] segments;
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

std::wstring FileCodec::DecodeAFM(const std::string& inputPath) const
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

// END IMPLEMENTATION