#pragma once

#include <string>
#include <vector>
#include <map>
#include <set> // makes ordered set
#include <algorithm> // sorting
#include <cstdint> // for int8_t, int16_t ...
#include <utility> // for std::pair

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

/* 
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

// Ariphmetical encoding

std::wstring CodecAFMTxtOnly::EncodeAFM(const std::wstring& str) const
{
    auto encode = [](const std::wstring& str) {
        // initialize sorted alphabet and sorted frequencies
        wchar_t* alphabet = Alphabet(str);
        int size = wcslen(alphabet);
        std::pair<wchar_t, double>* frequencies = CharFrequencyPairs(alphabet, size, str);
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
            result.push_back(frequencies[i].first);
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

// Huffman encoding

std::wstring CodecHUFTxtOnly::EncodeHUF(const std::wstring& str) const
{
    // initialize alphabet and char-frequency pairs sorted by char
    wchar_t* alphabet = Alphabet(str);
    int alphabetSize = wcslen(alphabet);
    std::pair<wchar_t, double>* charFrequencies = CharFrequencyPairs(alphabet, alphabetSize, str);
    // sort by frequencies
    std::sort(charFrequencies, charFrequencies + alphabetSize, [](const std::pair<wchar_t, double>& a, const std::pair<wchar_t, double>& b) {
        return a.second < b.second;
    });

    // build Huffman tree
    HuffmanTreeTest tree = BuildHuffmanTreeTest(charFrequencies, alphabetSize);
    // get Huffman codes
    std::vector<std::pair<wchar_t, std::wstring>> huffmanCodes = GetHuffmanCodes(tree, alphabetSize);
    // sort codes by chars
    std::sort(huffmanCodes.begin(), huffmanCodes.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });
    // make map of codes for fast access
    std::map<wchar_t, std::wstring> huffmanCodesMap(huffmanCodes.begin(), huffmanCodes.end());

    // make result string
    std::wstring encodedStr;
    // write alphabet size
    encodedStr += std::to_wstring(alphabetSize) + L'\n';
    // write alphabet
    for (int i = 0; i < alphabetSize; ++i) {
        encodedStr += huffmanCodes[i].first;
    }
    encodedStr += L'\n';
    // write huffman codes
    for (int i = 0; i < alphabetSize; ++i) {
        encodedStr += (huffmanCodes[i].second) + L' ';
    }
    encodedStr += L'\n';
    // write length of the string
    encodedStr += std::to_wstring(str.size()) + L'\n';
    // write encoded string
    for (size_t i = 0; i < str.size(); ++i) {
        encodedStr += huffmanCodesMap[str[i]];
    }

    delete[] alphabet; delete[] charFrequencies;
    return encodedStr;
}

 */
// END IMPLEMENTATION