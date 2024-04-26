#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility> // for std::pair

// START

struct HuffmanNode {
    std::u32string chars;
    double freq;
    HuffmanNode *left, *right;
    uint8_t height;

    HuffmanNode() = default;
    HuffmanNode(const std::u32string& chars, const double& freq, HuffmanNode* left, HuffmanNode* right, uint8_t height) : 
        chars(chars), freq(freq), left(left), right(right), height(height) {}
    HuffmanNode(const char32_t& c, const double& freq, HuffmanNode* left, HuffmanNode* right, uint8_t height) : 
        chars(std::u32string(1, c)), freq(freq), left(left), right(right), height(height) {}
    bool operator<(const HuffmanNode& other) const {
        if (freq < other.freq) { return true; }
        return false;
    }
};

class HuffmanTree {
private:
    void fillHuffmanCodes(HuffmanNode* node, const std::string& currentCode, std::map<char32_t, std::string>& huffmanCodes) {
        if (node->left == nullptr) { // also means that node.right == nullptr
            // write the code of current character
            huffmanCodes[node->chars[0]] = currentCode;
            return;
        }
        fillHuffmanCodes(node->left, currentCode + "0", huffmanCodes);
        fillHuffmanCodes(node->right, currentCode + "1", huffmanCodes);
    }
    HuffmanNode* root;
public:
    HuffmanTree(HuffmanNode* root) : root(root) {}
    uint8_t GetHeight() { return root->height; }
    friend std::map<char32_t, std::string> GetHuffmanCodes(HuffmanTree& tree, const size_t& alphabetSize);
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

HuffmanTree BuildHuffmanTree(std::vector<std::pair<char32_t, double>> sortedCharFrequencies, const size_t& alphabetSize) {
    std::vector<HuffmanNode*> freeNodesVector;
    freeNodesVector.reserve(alphabetSize * 2); // use size*2 as a maximum possible number of nodes

    // fill freeNodesVector
    for (uint32_t i = 0; i < alphabetSize; ++i) {
        freeNodesVector.push_back(new HuffmanNode(sortedCharFrequencies[i].first, sortedCharFrequencies[i].second, nullptr, nullptr, 1));
    }

    HuffmanNode *left, *right, *parent;
    
    // special case
    if (alphabetSize == 1) {
        parent = new HuffmanNode(sortedCharFrequencies[0].first, 1.0, nullptr, nullptr, 1);
    } else if (alphabetSize == 0) {
        parent = new HuffmanNode(' ', 0.0, nullptr, nullptr, 0);
    }
    // build Huffman tree
    while(freeNodesVector.size() > 1) {
        left = freeNodesVector[0];
        right = freeNodesVector[1];
        parent = new HuffmanNode(left->chars + right->chars, left->freq + right->freq, left, right, std::max(left->height, right->height) + 1);
        
        // remove right and left nodes from freeNodes
        for (size_t i = 2; i < freeNodesVector.size(); ++i) {
            freeNodesVector[i - 2] = freeNodesVector[i];
        }
        freeNodesVector.resize(freeNodesVector.size() - 2);

        size_t ind;
        // insert parent into freeNodes
        for (ind = 0; ind < freeNodesVector.size(); ++ind) {
            if (*parent < *(freeNodesVector[ind])) {
                HuffmanNode* temp;
                // insert parent before freeNodes[i]
                for (size_t j = ind; j < freeNodesVector.size(); ++j) {
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

    return HuffmanTree(parent);
}

std::map<char32_t, std::string> GetHuffmanCodes(HuffmanTree& tree, const size_t& alphabetSize) {
    std::map<char32_t, std::string> huffmanCodes;
    
    if (alphabetSize == 1) {
        // special case
        huffmanCodes[tree.root->chars[0]] = "0";
    } else {
        tree.fillHuffmanCodes(tree.root, "", huffmanCodes);
    }

    return huffmanCodes;
}

// END