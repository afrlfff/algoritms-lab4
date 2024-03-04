#pragma once

#include <string>
#include <fstream>
#include <map>
#include <set> // makes ordered set
#include <algorithm> // sorting


std::string Alphabet(const std::string& str); // return ordered alphabet from the string
std::string Encode(const std::string& key, const std::string& str);
std::string Encode(const std::string& key, const std::string& inputPath, const std::string& outputPath);
std::string EncodeRLE(const std::string& str); // Run-length encoding
std::string EncodeMTF(const std::string& str); // Move-to-front encoding

std::string Alphabet(const std::string& str)
{
    const char* charStr = str.c_str();
    std::set<char> charsSet(charStr, charStr + strlen(charStr));

    char* alphabet = new char[charsSet.size() + 1];
    int ind = 0;
    for (char c : charsSet) { 
        alphabet[ind++] = c;
    }
    alphabet[ind] = '\0'; // end of string

    std::sort(alphabet, alphabet + ind);

    return std::string(alphabet);
}

std::string Encode(const std::string& key, const std::string& str)
{
    if (key == "RLE") {
        return EncodeRLE(str);
    } else if (key == "MTF") {
        return EncodeMTF(str);
    } else {
        std::cout << "Error: The key " << key << " doesn't exist." << std::endl;
        return "";
    }
}

std::string Encode(const std::string& key, const std::string& inputPath, const std::string& outputPath)
{
    std::ifstream in(inputPath, std::ios::binary);
    std::ofstream out(outputPath, std::ios::binary);

    std::string inputStr;
    while(1) {
        char c = (char)in.get();
        if (in.eof()) break;
        inputStr += c;
    }

    std::string outputStr = Encode(key, inputStr);
    for(int i = 0; i < outputStr.size(); i++) {
        out << outputStr[i];
    }

    in.close();
    out.close();
    return outputStr;
}

std::string EncodeRLE(const std::string& str)
{
    std::string newStr;

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::string uniqueSeq(1, str[0]); // last sequence of unique characters

    // show if previous character was part of sequence
    bool flag = false;

    char prev = str[0]; // previous character

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
                newStr.append(std::to_string(-1 * countUnique) + uniqueSeq);

                countUnique = 1;
            }

            if (flag) { countIdent = 1; flag = false; } 
            else { countIdent++; }
            
            countUnique = 0;
            uniqueSeq = "";
        }
        else 
        {
            // record last sequence of identical symbols if it exists
            if (countIdent > 1) {
                if (countIdent >= 9) {
                    for (int i = 0; i < (countIdent / 9); i++) {
                        newStr.push_back('9');
                        newStr.push_back(prev);
                    }
                }
                if (countIdent % 9 != 0) {
                    newStr.push_back(('0' + (countIdent % 9)));
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
                uniqueSeq += str[i];
            }
            countIdent = 1;

            // limit length of sequence
            if (countUnique == 9) {
                newStr.append(std::to_string(-1 * countUnique) + uniqueSeq);
                flag = true;
                countUnique = 0;
                uniqueSeq = "";
            }
        }
        prev = str[i];
    }

    // record last sequence which was lost in the loop
    if (countIdent > 1) {
        if (countIdent >= 9) {
            for (int i = 0; i < (countIdent / 9); i++) {
                newStr.push_back('9');
                newStr.push_back(prev);
            }
        }
        if (countIdent % 9 != 0) {
            newStr.push_back(('0' + (countIdent % 9)));
            newStr.push_back(prev);
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        newStr.append(std::to_string(-1 * countUnique) + uniqueSeq);
    }

    return newStr;
}

std::string EncodeMTF(const std::string& str)
{
    std::string alphabet = Alphabet(str);
    int alphabetLength = alphabet.size();

    std::string encodedArray;

    // write alphabet
    for (int i = 0; i < alphabetLength; i++) {
        encodedArray += alphabet[i];
    }
    encodedArray += '\n';

    // move-to-front
    for (int i = 0; i < str.size(); i++) {
        int index = alphabet.find(str[i]);
        encodedArray += std::to_string(index) + " ";

        char temp = alphabet[0];
        for (int j = 1; j <= index; j++) {
            char temp2 = alphabet[j];
            alphabet[j] = temp;
            temp = temp2;
        }
        alphabet[0] = temp;
    }

    return encodedArray;
}

// END