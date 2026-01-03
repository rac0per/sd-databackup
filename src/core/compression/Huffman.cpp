#include "Huffman.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <map>
#include <queue>
#include <cstring>
#include <cstdint>
#include <functional>
#include <bitset>
namespace backup::core::compression
{
    HuffmanNode::HuffmanNode(uint8_t data, uint64_t frequency) : data(data), frequency(frequency), left(nullptr), right(nullptr) {}
    HuffmanNode::~HuffmanNode()
    {
        delete left;
        delete right;
    }
    bool CompareHuffmanNodes::operator()(const HuffmanNode *lhs, const HuffmanNode *rhs)
    {
        return lhs->frequency > rhs->frequency;
    }
    HuffmanNode *Huffman::buildHuffmanTree(const std::map<uint8_t, uint64_t> &frequencyMap)
    {
        std::priority_queue<HuffmanNode *, std::vector<HuffmanNode *>, CompareHuffmanNodes> pq;
        for (const auto &pair : frequencyMap)
        {
            pq.push(new HuffmanNode(pair.first, pair.second));
        }
        if (pq.empty())
        {
            return new HuffmanNode(0, 0);
        }
        if (pq.size() == 1)
        {
            HuffmanNode *singleNode = pq.top();
            pq.pop();
            HuffmanNode *parent = new HuffmanNode(0, singleNode->frequency);
            parent->left = singleNode;
            parent->right = nullptr;
            return parent;
        }
        while (pq.size() > 1)
        {
            HuffmanNode *left = pq.top();
            pq.pop();
            HuffmanNode *right = pq.top();
            pq.pop();
            HuffmanNode *parent = new HuffmanNode(0, left->frequency + right->frequency);
            parent->left = left;
            parent->right = right;
            pq.push(parent);
        }
        return pq.top();
    }
    void Huffman::generateHuffmanCodes(HuffmanNode *root, const std::string &code, std::map<uint8_t, std::string> &huffmanCodes)
    {
        if (root == nullptr)
        {
            return;
        }
        if (root->left == nullptr && root->right == nullptr)
        {
            huffmanCodes[root->data] = code.empty() ? "0" : code;
            return;
        }
        generateHuffmanCodes(root->left, code + "0", huffmanCodes);
        if (root->right != nullptr)
        {
            generateHuffmanCodes(root->right, code + "1", huffmanCodes);
        }
    }
    std::map<uint8_t, uint64_t> Huffman::calculateFrequencies(const std::vector<uint8_t> &data)
    {
        std::map<uint8_t, uint64_t> frequencyMap;
        for (uint8_t byte : data)
        {
            frequencyMap[byte]++;
        }
        return frequencyMap;
    }
    std::vector<uint8_t> Huffman::compressData(const std::vector<uint8_t> &input, const std::map<uint8_t, std::string> &huffmanCodes)
    {
        std::string bitString;
        for (uint8_t byte : input)
        {
            bitString += huffmanCodes.at(byte);
        }
        std::vector<uint8_t> compressedData;
        for (size_t i = 0; i < bitString.size(); i += 8)
        {
            std::string byteString = bitString.substr(i, 8);
            while (byteString.size() < 8)
            {
                byteString += "0";
            }
            uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 2));
            compressedData.push_back(byte);
        }
        compressedData.push_back(static_cast<uint8_t>(bitString.size() % 8));
        return compressedData;
    }
    std::vector<uint8_t> Huffman::decompressData(const std::vector<uint8_t> &input, HuffmanNode *root, size_t originalSize)
    {
        std::vector<uint8_t> decompressedData;
        if (input.empty() || root == nullptr)
        {
            return decompressedData;
        }
        HuffmanNode *current = root;
        uint8_t remainingBits = input.back();
        for (size_t i = 0; i < input.size() - 1; ++i)
        {
            uint8_t byte = input[i];
            for (int j = 7; j >= 0; --j)
            {
                if (i == input.size() - 2 && remainingBits != 0 && j < 8 - remainingBits)
                {
                    break;
                }
                bool bit = (byte >> j) & 1;
                if (bit)
                {
                    current = current->right;
                }
                else
                {
                    current = current->left;
                }
                if (current->left == nullptr && current->right == nullptr)
                {
                    decompressedData.push_back(current->data);
                    current = root;
                    if (decompressedData.size() == originalSize)
                    {
                        return decompressedData;
                    }
                }
            }
        }
        return decompressedData;
    }
    void Huffman::saveHuffmanTree(HuffmanNode *root, std::ofstream &output)
    {
        if (root == nullptr)
        {
            return;
        }
        if (root->left == nullptr && root->right == nullptr)
        {
            output.put(1);
            output.put(root->data);
        }
        else
        {
            output.put(0);
            saveHuffmanTree(root->left, output);
            saveHuffmanTree(root->right, output);
        }
    }
    HuffmanNode *Huffman::loadHuffmanTree(std::ifstream &input)
    {
        uint8_t flag;
        if (!input.read(reinterpret_cast<char *>(&flag), 1))
        {
            return nullptr;
        }
        if (flag == 1)
        {
            uint8_t data;
            if (!input.read(reinterpret_cast<char *>(&data), 1))
            {
                return nullptr;
            }
            return new HuffmanNode(data, 0);
        }
        else
        {
            HuffmanNode *left = loadHuffmanTree(input);
            std::streampos currentPos = input.tellg();
            uint8_t nextFlag;
            if (input.read(reinterpret_cast<char *>(&nextFlag), 1))
            {
                input.seekg(currentPos);
                HuffmanNode *right = loadHuffmanTree(input);
                HuffmanNode *parent = new HuffmanNode(0, 0);
                parent->left = left;
                parent->right = right;
                return parent;
            }
            else
            {
                input.seekg(currentPos);
                HuffmanNode *parent = new HuffmanNode(0, 0);
                parent->left = left;
                parent->right = nullptr;
                return parent;
            }
        }
    }
    std::vector<uint8_t> Huffman::compress(const std::vector<uint8_t> &data)
    {
        std::map<uint8_t, uint64_t> frequencyMap = calculateFrequencies(data);
        HuffmanNode *root = buildHuffmanTree(frequencyMap);
        std::map<uint8_t, std::string> huffmanCodes;
        generateHuffmanCodes(root, "", huffmanCodes);
        std::vector<uint8_t> compressedData = compressData(data, huffmanCodes);
        std::vector<uint8_t> result;
        size_t treeStartPos = 0;
        uint32_t treeSize = 0;
        result.insert(result.end(), reinterpret_cast<uint8_t *>(&treeSize), reinterpret_cast<uint8_t *>(&treeSize) + sizeof(treeSize));
        std::vector<uint8_t> treeBuffer;
        auto saveTreeToBuffer = [&treeBuffer](auto &&self, HuffmanNode *node) -> void
        {
            if (node == nullptr)
            {
                return;
            }
            if (node->left == nullptr && node->right == nullptr)
            {
                treeBuffer.push_back(1);
                treeBuffer.push_back(node->data);
            }
            else
            {
                treeBuffer.push_back(0);
                self(self, node->left);
                self(self, node->right);
            }
        };
        saveTreeToBuffer(saveTreeToBuffer, root);
        treeSize = static_cast<uint32_t>(treeBuffer.size());
        std::memcpy(result.data(), &treeSize, sizeof(treeSize));
        result.insert(result.end(), treeBuffer.begin(), treeBuffer.end());
        result.insert(result.end(), compressedData.begin(), compressedData.end());
        delete root;
        return result;
    }
    std::vector<uint8_t> Huffman::decompress(const std::vector<uint8_t> &data, size_t originalSize)
    {
        if (data.size() < sizeof(uint32_t))
        {
            return std::vector<uint8_t>();
        }
        uint32_t treeSize = *reinterpret_cast<const uint32_t *>(data.data());
        if (data.size() < sizeof(uint32_t) + treeSize)
        {
            return std::vector<uint8_t>();
        }
        size_t treeStartPos = sizeof(uint32_t);
        size_t treeEndPos = treeStartPos + treeSize;
        size_t currentPos = treeStartPos;
        auto buildTreeFromBuffer = [&data, &currentPos, &treeEndPos](auto &&self) -> HuffmanNode *
        {
            if (currentPos >= treeEndPos)
            {
                return nullptr;
            }
            uint8_t flag = data[currentPos++];
            if (flag == 1)
            {
                if (currentPos >= treeEndPos)
                {
                    return nullptr;
                }
                uint8_t byteData = data[currentPos++];
                return new HuffmanNode(byteData, 0);
            }
            else
            {
                HuffmanNode *left = self(self);
                HuffmanNode *right = self(self);
                HuffmanNode *parent = new HuffmanNode(0, 0);
                parent->left = left;
                parent->right = right;
                return parent;
            }
        };
        HuffmanNode *root = buildTreeFromBuffer(buildTreeFromBuffer);
        if (root == nullptr)
        {
            return std::vector<uint8_t>();
        }
        std::vector<uint8_t> compressedData(data.begin() + treeEndPos, data.end());
        std::vector<uint8_t> decompressedData = decompressData(compressedData, root, originalSize);
        delete root;
        return decompressedData;
    }
} 