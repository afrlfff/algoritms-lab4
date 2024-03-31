#pragma once

#include <string>
#include <vector>

struct HuffmanNodeTest {
    std::wstring chars;
    double freq;
    HuffmanNodeTest *left, *right;

    HuffmanNodeTest() = default;
    HuffmanNodeTest(const std::wstring& chars, const double& freq, HuffmanNodeTest* left, HuffmanNodeTest* right) : 
        chars(chars), freq(freq), left(left), right(right) {}
    HuffmanNodeTest(const wchar_t& c, const double& freq, HuffmanNodeTest* left, HuffmanNodeTest* right) : 
        chars(std::wstring(1, c)), freq(freq), left(left), right(right) {}
    bool operator<=(const HuffmanNodeTest& other) const {
        if (freq < other.freq || (freq - other.freq) < 1e-14) {
            return true;
        }
        return false;
    }
};

class HuffmanTreeTest {
    void fillHuffmanCodes(HuffmanNodeTest* node, std::wstring currentCode, std::vector<std::pair<wchar_t, std::wstring>>& huffmanCodes) {
        if (node->left == nullptr) { // also means that node.right == nullptr
            // write the code of current character
            huffmanCodes.push_back(std::pair<wchar_t, std::wstring>(node->chars[0], currentCode));
            return;
        }
        fillHuffmanCodes(node->left, currentCode + L'0', huffmanCodes);
        fillHuffmanCodes(node->right, currentCode + L'1', huffmanCodes);
    }
public:
    HuffmanNodeTest* root;
    HuffmanTreeTest(HuffmanNodeTest* root) : root(root) {}
    friend std::vector<std::pair<wchar_t, std::wstring>> GetHuffmanCodes(HuffmanTreeTest& tree, const int& alphabetSize);
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

HuffmanTreeTest BuildHuffmanTreeTest(const std::pair<wchar_t, double>* charFrequencies, const int& alphabetSize) {
    // initialize freeNodes
    HuffmanNodeTest** freeNodes = new HuffmanNodeTest*[alphabetSize * 2]; // use size * 2 as a maximum possible number of nodes
    for (int i = 0; i < alphabetSize; ++i) {
        freeNodes[i] = new HuffmanNodeTest(charFrequencies[i].first, charFrequencies[i].second, nullptr, nullptr);
    } for (int i = alphabetSize; i < alphabetSize * 2; ++i) {
        freeNodes[i] = new HuffmanNodeTest(L"-", 0.0, nullptr, nullptr);
    }

    // build Huffman tree
    int freeNodesSize = alphabetSize;
    HuffmanNodeTest *left, *right, *parent;
    
    // special case
    if (freeNodesSize == 1) {
        parent = new HuffmanNodeTest(charFrequencies[0].first, 1.0, nullptr, nullptr);
    }
    while(freeNodesSize > 1) {
        left = freeNodes[0];
        right = freeNodes[1];
        parent = new HuffmanNodeTest(left->chars + right->chars, left->freq + right->freq, left, right);
        
        // remove right and left nodes from freeNodes
        for (int i = 2; i < freeNodesSize; ++i) {
            freeNodes[i - 2] = freeNodes[i];
        }
        freeNodesSize -= 2;

        // insert parent into freeNodes
        for (int i = 0; i <= freeNodesSize; ++i) {
            if (*parent <= *(freeNodes[i])) {
                // insert before freeNodes[i]
                for (int j = i; j < freeNodesSize + 1; ++j) {
                    HuffmanNodeTest* temp = freeNodes[j];
                    freeNodes[j] = parent;
                    parent = temp; // will use parent as a temp2
                }
                break;
            }
            if (i == freeNodesSize) {
                // if parent should be inserted at the end
                freeNodes[i] = parent;
            }
        }
        ++freeNodesSize;
    }
    
    // deallocate memory
    for (int i = 0; i < 2 * alphabetSize; ++i) {
        if (freeNodes[i]->chars == L"-") {
            delete freeNodes[i];
        }
    }
    delete[] freeNodes;

    return HuffmanTreeTest(parent);
}

std::vector<std::pair<wchar_t, std::wstring>> GetHuffmanCodes(HuffmanTreeTest& tree, const int& alphabetSize) {
    std::vector<std::pair<wchar_t, std::wstring>> huffmanCodes;
    huffmanCodes.reserve(alphabetSize); // preallocate memory for efficiency
    
    // special case
    if (alphabetSize == 1) {
        huffmanCodes.push_back(std::pair<wchar_t, std::wstring>(tree.root->chars[0], L"0"));
        return huffmanCodes;
    }

    tree.fillHuffmanCodes(tree.root, L"", huffmanCodes);
    return huffmanCodes;
}

// END