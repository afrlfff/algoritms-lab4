#pragma once

#include <string>
#include <vector>
#include <utility> // for std::pair

// Huffman

struct HuffmanNode {
    std::wstring chars;
    double freq;
    HuffmanNode *left, *right;

    HuffmanNode() = default;
    HuffmanNode(const std::wstring& chars, const double& freq, HuffmanNode* left, HuffmanNode* right) : 
        chars(chars), freq(freq), left(left), right(right) {}
    HuffmanNode(const wchar_t& c, const double& freq, HuffmanNode* left, HuffmanNode* right) : 
        chars(std::wstring(1, c)), freq(freq), left(left), right(right) {}
    bool operator<=(const HuffmanNode& other) const {
        if (freq < other.freq || (freq - other.freq) < 1e-14) {
            return true;
        }
        return false;
    }
};

class HuffmanTree {
    void fillHuffmanCodes(HuffmanNode* node, std::string currentCode, std::vector<std::pair<wchar_t, std::string>>& huffmanCodes) {
        if (node->left == nullptr) { // also means that node.right == nullptr
            // write the code of current character
            huffmanCodes.push_back(std::make_pair(node->chars[0], currentCode));
            return;
        }
        fillHuffmanCodes(node->left, currentCode + '0', huffmanCodes);
        fillHuffmanCodes(node->right, currentCode + '1', huffmanCodes);
    }
public:
    HuffmanNode* root;
    HuffmanTree(HuffmanNode* root) : root(root) {}
    friend std::vector<std::pair<wchar_t, std::string>> GetHuffmanCodes(HuffmanTree& tree, const int& alphabetSize);
    void clearNode(HuffmanNode* node) {
        if (node->left != nullptr) {
            clearNode(node->left);
        } if (node->right != nullptr) {
            clearNode(node->right);
        }
        delete node;
    }
    ~HuffmanTree() { clearNode(root); }
};

HuffmanTree BuildHuffmanTree(const std::pair<wchar_t, double>* charFrequencies, const int& alphabetSize) {
    // initialize freeNodes
    HuffmanNode** freeNodes = new HuffmanNode*[alphabetSize * 2]; // use size * 2 as a maximum possible number of nodes
    for (int i = 0; i < alphabetSize; ++i) {
        freeNodes[i] = new HuffmanNode(charFrequencies[i].first, charFrequencies[i].second, nullptr, nullptr);
    } for (int i = alphabetSize; i < alphabetSize * 2; ++i) {
        freeNodes[i] = new HuffmanNode(L"-", 0.0, nullptr, nullptr);
    }

    // build Huffman tree
    int freeNodesSize = alphabetSize;
    HuffmanNode *left, *right, *parent;
    
    // special case
    if (freeNodesSize == 1) {
        parent = new HuffmanNode(charFrequencies[0].first, 1.0, nullptr, nullptr);
    }
    while(freeNodesSize > 1) {
        left = freeNodes[0];
        right = freeNodes[1];
        parent = new HuffmanNode(left->chars + right->chars, left->freq + right->freq, left, right);
        
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
                    HuffmanNode* temp = freeNodes[j];
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

    return HuffmanTree(parent);
}

std::vector<std::pair<wchar_t, std::string>> GetHuffmanCodes(HuffmanTree& tree, const int& alphabetSize) {
    std::vector<std::pair<wchar_t, std::string>> huffmanCodes;
    huffmanCodes.reserve(alphabetSize); // preallocate memory for efficiency
    
    // special case
    if (alphabetSize == 1) {
        huffmanCodes.push_back(std::make_pair(tree.root->chars[0], "0"));
        return huffmanCodes;
    }

    tree.fillHuffmanCodes(tree.root, "", huffmanCodes);
    return huffmanCodes;
}


// END