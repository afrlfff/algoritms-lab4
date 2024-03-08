#pragma once

#include <string>
#include <fstream>
#include <map>
#include <set> // makes ordered set
#include <algorithm> // sorting

#include "ReadFileUtf8.h"
#include "WriteFileUtf8.h"

wchar_t* Alphabet(const std::wstring& str); // return ordered alphabet from the string
std::wstring Encode(const std::string& key, const std::wstring& str);
std::wstring Encode(const std::string& key, const std::string& inputPath, const std::string& outputPath);
std::wstring EncodeRLE(const std::wstring& str); // Run-length encoding
std::wstring EncodeMTF(const std::wstring& str); // Move-to-front encoding
std::wstring EncodeBWT(const std::wstring& str); // Burrows-Wheeler transform


wchar_t* Alphabet(const std::wstring& str)
{
    const wchar_t* charStr = str.c_str();
    std::set<wchar_t> charsSet(charStr, charStr + wcslen(charStr));

    wchar_t* alphabet = new wchar_t[charsSet.size() + 1];
    int ind = 0;
    for (wchar_t c : charsSet) { 
        alphabet[ind++] = c;
    }
    alphabet[ind] = '\0'; // end of string

    std::sort(alphabet, alphabet + ind);

    return alphabet;
}

std::wstring Encode(const std::string& key, const std::wstring& str)
{
    if (key == "RLE") {
        return EncodeRLE(str);
    } else if (key == "MTF") {
        return EncodeMTF(str);
    } else if (key == "BWT") {
        return EncodeBWT(str);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return L"";
    }
}

std::wstring Encode(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    std::wstring inputStr = ReadFileUtf8(inputPath);
    std::wstring outputStr = Encode(key, inputStr);

    WriteFileUtf8(outputStr, outputPath);
    
    return outputStr;
}

std::wstring EncodeRLE(const std::wstring& str)
{
    std::wstring newStr = L"";

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::wstring uniqueSeq(1, str[0]); // last sequence of unique characters

    // show if previous character was part of sequence
    bool flag = false;

    wchar_t prev = str[0]; // previous character

    // start RLE
    for (int i = 1; i < str.size(); i++)
    {
        if (str[i] == prev) 
        {
            // record last sequence of unique symbols if it exists
            if (countUnique > 1) {
                uniqueSeq.pop_back(); // because "prev" was read as unique
                countUnique--; // because "prev" was read as unique

                countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
                newStr += (std::to_wstring(-1 * countUnique) + uniqueSeq);

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { countIdent++; }
            
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

                countUnique++;
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
            for (int i = 0; i < (countIdent / 9); i++) {
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

std::wstring EncodeMTF(const std::wstring& str)
{
    // return the index of the character in the alphabet
    auto getPosition = [](const wchar_t* array, size_t size, wchar_t c) {
        const wchar_t* end = array + size;
        const wchar_t* match = std::find(array, end, c);
        return (end == match) ? -1 : (match - array);
    };

    wchar_t* alphabet = Alphabet(str);
    size_t alphabetLength = wcslen(alphabet);

    std::wstring encodedStr;

    // write alphabet
    for (size_t i = 0; i < alphabetLength; i++) {
        encodedStr.push_back(alphabet[i]);
    }
    encodedStr += '\n';

    // move-to-front
    for (size_t i = 0; i < str.size(); i++) {
        size_t index = getPosition(alphabet, alphabetLength, str[i]);
        encodedStr += std::to_wstring(index);
        encodedStr.push_back(L' ');

        // shift
        wchar_t temp = alphabet[0];
        for (size_t j = 1; j <= index; j++) {
            wchar_t temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;
    }

    return encodedStr;
}

std::wstring EncodeBWT(const std::wstring& str)
{
    size_t permutationsLength = str.size();
    std::wstring* permutations = new std::wstring[permutationsLength];

    // generate permutations
    permutations[0] = str;
    for (size_t i = 1; i < permutationsLength; i++) {
        permutations[i] = str.substr(i, permutationsLength - i) + str.substr(0, i);
    }
    
    // sort permutations
    std::sort(permutations, permutations + permutationsLength);

    // return result
    std::wstring result = L"";
    size_t indexOfOrignal;
    for (size_t i = 0; i < permutationsLength; i++) {
        result += permutations[i][permutationsLength - 1];
        if (permutations[i] == str) indexOfOrignal = i;
    }
    result.insert(0, std::to_wstring(indexOfOrignal) + L"\n");

    delete[] permutations;
    return result;
}

// END