// !!!!!!!!!!!!
// File just for testing compression algorithms
// not for using in a program
// !!!!!!!!!!!!

#pragma once
#include <string>
#include <vector>
#include <map>
#include <set> // makes ordered set
#include <algorithm> // sorting
#include <cstdint> // for int8_t, int16_t ...
#include <utility> // for std::pair

#include "SuffixArray.h"

// Text tools

std::string GetAlphabet(const std::string& str)
{
    const char* charStr = str.c_str();
    std::set<char32_t> charsSet(charStr, charStr + str.size());

    std::string alphabet; alphabet.reserve(charsSet.size() + 1);

    for (char32_t c : charsSet) { 
        alphabet.push_back(c);
    }

    std::sort(alphabet.begin(), alphabet.end());
    return alphabet;
}

std::map<char, double> GetCharFrequenciesMap(const std::string& alphabet, const size_t& size, const std::string& str)
{
    std::map<char, double> charFrequencies;

    size_t countAll = 0;
    for (char c : str) {
        ++charFrequencies[c];
        ++countAll;
    } for (size_t i = 0; i < size; i++) {
        charFrequencies[alphabet[i]] /= static_cast<double>(countAll);
    }

    return charFrequencies;
}

// Huffman tree

struct HuffmanNodeTest {
    std::string chars;
    double freq;
    HuffmanNodeTest *left, *right;

    HuffmanNodeTest() = default;
    HuffmanNodeTest(const std::string& chars, const double& freq, HuffmanNodeTest* left, HuffmanNodeTest* right) : 
        chars(chars), freq(freq), left(left), right(right) {}
    HuffmanNodeTest(const char& c, const double& freq, HuffmanNodeTest* left, HuffmanNodeTest* right) : 
        chars(std::string(1, c)), freq(freq), left(left), right(right) {}
    bool operator<(const HuffmanNodeTest& other) const {
        // if (freq < other.freq || (freq - other.freq) < 1e-14) {
        if (freq < other.freq) {
            return true;
        }
        return false;
    }
};

class HuffmanTreeTest {
    void fillHuffmanCodes(HuffmanNodeTest* node, const std::string& currentCode, std::map<char, std::string>& huffmanCodes) {
        if (node->left == nullptr) { // also means that node.right == nullptr
            // write the code of current character
            huffmanCodes[node->chars[0]] = currentCode;
            return;
        }
        fillHuffmanCodes(node->left, currentCode + "0", huffmanCodes);
        fillHuffmanCodes(node->right, currentCode + "1", huffmanCodes);
    }
public:
    HuffmanNodeTest* root;
    HuffmanTreeTest(HuffmanNodeTest* root) : root(root) {}
    friend std::map<char, std::string> GetHuffmanCodes(HuffmanTreeTest& tree, const int& alphabetSize);
    void clearNode(HuffmanNodeTest* node) {
        if (node->left != nullptr) {
            clearNode(node->left);
        } if (node->right != nullptr) {
            clearNode(node->right);
        }
        delete node;
    }
    ~HuffmanTreeTest() { clearNode(root); }
};

HuffmanTreeTest BuildHuffmanTreeTest(std::vector<std::pair<char, double>> charFrequencies, const int& alphabetSize) {
    std::vector<HuffmanNodeTest*> freeNodesVector;
    freeNodesVector.reserve(alphabetSize * 2); // use size*2 as a maximum possible number of nodes

    // fill freeNodesVector
    for (int i = 0; i < alphabetSize; ++i) {
        freeNodesVector.push_back(new HuffmanNodeTest(charFrequencies[i].first, charFrequencies[i].second, nullptr, nullptr));
    }

    HuffmanNodeTest *left, *right, *parent;
    
    // special case
    if (alphabetSize == 1) {
        parent = new HuffmanNodeTest(charFrequencies[0].first, 1.0, nullptr, nullptr);
    }
    // build Huffman tree
    while(freeNodesVector.size() > 1) {
        left = freeNodesVector[0];
        right = freeNodesVector[1];
        parent = new HuffmanNodeTest(left->chars + right->chars, left->freq + right->freq, left, right);
        
        // remove right and left nodes from freeNodes
        for (int i = 2; i < freeNodesVector.size(); ++i) {
            freeNodesVector[i - 2] = freeNodesVector[i];
        
        }
        freeNodesVector.resize(freeNodesVector.size() - 2);

        int ind;
        // insert parent into freeNodes
        for (ind = 0; ind < freeNodesVector.size(); ++ind) {
            if (*parent < *(freeNodesVector[ind])) {
                HuffmanNodeTest* temp;
                // insert parent before freeNodes[i]
                for (int j = ind; j < freeNodesVector.size(); ++j) {
                    temp = freeNodesVector[j];
                    freeNodesVector[j] = parent;
                    parent = temp; // will use parent as a temp2
                }
                freeNodesVector.push_back(parent);
                break;
            }
        }
        // if parent should be inserted at the end
        if (ind == freeNodesVector.size()) {
            freeNodesVector.push_back(parent);
        }
    }

    return HuffmanTreeTest(parent);
}

std::map<char, std::string> GetHuffmanCodes(HuffmanTreeTest& tree, const int& alphabetSize) {
    std::map<char, std::string> huffmanCodes;
    
    // special case
    if (alphabetSize == 1) {
        huffmanCodes[tree.root->chars[0]] = "0";
    } else {
        tree.fillHuffmanCodes(tree.root, "", huffmanCodes);
    }
    
    return huffmanCodes;
}

// Codecs

std::string EncodeRLE_toString(const std::string& inputStr)
{
    std::string encodedStr;

    int countIdent = 1; // current count of repeating identical characters
    int countUnique = 1; // current count of repeating unique characters
    std::string uniqueSeq(1, inputStr[0]); // last sequence of unique characters
    bool flag = false; // show if previous character was part of sequence
    char32_t prev = inputStr[0]; // previous character

    int maxPossibleNumber = 127; // maximum possible value of int8_t

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
                encodedStr += (std::to_string(-1 * countUnique) + uniqueSeq);

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
                    for (int i = 0; i < (countIdent / maxPossibleNumber); i++) {
                        encodedStr += (std::to_string(maxPossibleNumber) + std::string(1, prev));
                    }
                }
                if (countIdent % maxPossibleNumber != 0) {
                    encodedStr += (std::to_string(countIdent % maxPossibleNumber) + std::string(1, prev));
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
                encodedStr += (std::to_string(-1 * countUnique) + uniqueSeq);
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
                encodedStr += (std::to_string(maxPossibleNumber) + std::string(1, prev));
            }
        }
        if (countIdent % maxPossibleNumber != 0) {
            encodedStr += (std::to_string(countIdent % maxPossibleNumber) + std::string(1, prev));
        }
    }
    if (countUnique > 0) { 
        countUnique = (countUnique == 1) ? -1 : countUnique; // to avoid -1
        encodedStr += (std::to_string(-1 * countUnique) + uniqueSeq);
    }

    return encodedStr;
}

std::string EncodeHA_toString(const std::string& inputStr)
{
    std::string alphabet = GetAlphabet(inputStr);
    int alphabetSize = alphabet.size();
    std::map<char, double> charFrequenciesMap = GetCharFrequenciesMap(alphabet, alphabetSize, inputStr);
    std::vector<std::pair<char, double>> charFrequenciesVector(charFrequenciesMap.begin(), charFrequenciesMap.end());
    // sort by vector frequencies
    std::sort(charFrequenciesVector.begin(), charFrequenciesVector.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    HuffmanTreeTest tree = BuildHuffmanTreeTest(charFrequenciesVector, alphabetSize);
    std::map<char, std::string> huffmanCodesMap = GetHuffmanCodes(tree, alphabetSize);

    std::string encodedStr;
    // write alphabet size
    encodedStr += std::to_string(alphabetSize) + "\n";
    // write alphabet
    encodedStr += alphabet + "\n";
    // write huffman codes
    for (int i = 0; i < alphabetSize; ++i) {
        encodedStr += huffmanCodesMap[alphabet[i]] + " ";
    } encodedStr += "\n";
    // write length of the string
    encodedStr += std::to_string(inputStr.size()) + "\n";
    // write encoded string
    for (size_t i = 0; i < inputStr.size(); ++i) {
        encodedStr += huffmanCodesMap[inputStr[i]];
    }
    return encodedStr;
}

std::string EncodeBWTNaive_toString(const std::string& inputStr)
{
    std::vector<std::string> permutations; permutations.reserve(inputStr.size());
    for (size_t i = 0; i < inputStr.size(); ++i) {
        permutations.push_back(inputStr.substr(i) + inputStr.substr(0, i));
    }
    std::sort(permutations.begin(), permutations.end());

    std::string encodedStr; encodedStr.reserve(inputStr.size());
    for (size_t i = 0; i < inputStr.size(); ++i) {
        encodedStr.push_back(permutations[i][inputStr.size() - 1]);
    }
    return encodedStr;
}

std::string EncodeBWT_toString(const std::string& inputStr)
{
    std::string encodedStr; encodedStr.reserve(inputStr.size());

    std::vector<unsigned int> suffixArray = buildSuffixArray(inputStr);
    for (size_t i = 0; i < suffixArray.size(); ++i) {
        size_t ind = (suffixArray[i] > 0) ? (suffixArray[i] - 1) : (inputStr.size() - 1);
        encodedStr.push_back(inputStr[ind]);
    }
    return encodedStr;
}





struct LZ77_match {
    int offset;
    int length;
    char c;
    LZ77_match(int offset, int length, char c) : offset(offset), length(length), c(c) {}
};

// return start and length of the maximum prefix ({0, 0} if not found)
std::pair<int, int> findMaximumPrefix(const std::string inputStr, int stringPointer, const int SEARCH_BUFFER_SIZE, const int lOOKAHEAD_BUFFER_SIZE) {
    std::pair<int, int> prefixData = { 0, 0 };
    std::string prefix; prefix.reserve(lOOKAHEAD_BUFFER_SIZE);

    for (int i = 0; i < lOOKAHEAD_BUFFER_SIZE; ++i) {
        prefix.push_back(inputStr[stringPointer + i]);
        size_t prefixStart;

        // find prefix in the window
        if (stringPointer - SEARCH_BUFFER_SIZE < 0) {
            prefixStart = inputStr.substr(0, stringPointer).find(prefix);
        } else {
            prefixStart = inputStr.substr(stringPointer - SEARCH_BUFFER_SIZE, stringPointer).find(prefix);
        }

        if (prefixStart == std::string::npos) {
            return prefixData;
        } else {
            prefixData = { static_cast<int>(stringPointer - prefixStart), static_cast<int>(prefix.size()) };
        }
    }

    return prefixData;
}

std::string EncodeLZ77_toString(const std::string& inputStr) {
    const int SEARCH_BUFFER_SIZE = 32000;
    const int lOOKAHEAD_BUFFER_SIZE = 256; // maximum length of prefix
    int stringPointer = 0; // point at the first character in the string after hte window 

    // get all the matches
    std::vector<LZ77_match> matches;
    while (stringPointer < inputStr.size())
    {
        std::pair<int, int> prefixData = findMaximumPrefix(inputStr, stringPointer, SEARCH_BUFFER_SIZE, lOOKAHEAD_BUFFER_SIZE);
        if (prefixData.first == 0 && prefixData.second == 0) {
            matches.push_back(LZ77_match(0, 0, inputStr[stringPointer]));
            ++stringPointer;
        } else {
            matches.push_back(LZ77_match(prefixData.first, prefixData.second, '\0'));
            stringPointer += prefixData.second;
        }
    }

    //return matches;
    // encode matches
    std::string encodedStr;
    encodedStr += std::to_string(inputStr.size()) + "\n";
    for (int i = 0; i < matches.size(); ++i) {
        encodedStr += std::to_string(matches[i].offset) + " ";
    }
    encodedStr += "\n";
    for (int i = 0; i < matches.size(); ++i) {
        encodedStr += std::to_string(matches[i].length) + " ";
    }
    encodedStr += "\n";
    for (int i = 0; i < matches.size(); ++i) {
        encodedStr.push_back(matches[i].c);
    }
    return encodedStr;
}

// END IMPLEMENTATION
