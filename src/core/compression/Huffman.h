#pragma once
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <cstdint>
namespace backup::core::compression
{
    struct HuffmanNode
    {
        uint8_t data;       
        uint64_t frequency; 
        HuffmanNode *left;  
        HuffmanNode *right; 
        HuffmanNode(uint8_t data, uint64_t frequency);
        ~HuffmanNode();
    };
    struct CompareHuffmanNodes
    {
        bool operator()(const HuffmanNode *lhs, const HuffmanNode *rhs);
    };
    class Huffman
    {
    private:
        HuffmanNode *buildHuffmanTree(const std::map<uint8_t, uint64_t> &frequencyMap);
        void generateHuffmanCodes(HuffmanNode *root, const std::string &code, std::map<uint8_t, std::string> &huffmanCodes);
        std::map<uint8_t, uint64_t> calculateFrequencies(const std::vector<uint8_t> &data);
        std::vector<uint8_t> compressData(const std::vector<uint8_t> &input, const std::map<uint8_t, std::string> &huffmanCodes);
        std::vector<uint8_t> decompressData(const std::vector<uint8_t> &input, HuffmanNode *root, size_t originalSize);
        void saveHuffmanTree(HuffmanNode *root, std::ofstream &output);
        HuffmanNode *loadHuffmanTree(std::ifstream &input);
    public:
        std::vector<uint8_t> compress(const std::vector<uint8_t> &data);
        std::vector<uint8_t> decompress(const std::vector<uint8_t> &data, size_t originalSize);
    };
} 