#include "Compression.h"
#include "Huffman.h"
#include "LZ77.h"
#include <fstream>
#include <stdexcept>
#include <vector>
namespace backup::core::compression
{
    class HuffmanCompression : public Compression
    {
    public:
        void compress(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override
        {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open())
            {
                throw std::runtime_error("Failed to open input file: " + inputPath.string());
            }
            input.seekg(0, std::ios::end);
            size_t fileSize = input.tellg();
            input.seekg(0, std::ios::beg);
            std::vector<uint8_t> data(fileSize);
            input.read(reinterpret_cast<char *>(data.data()), fileSize);
            input.close();
            Huffman huffman;
            std::vector<uint8_t> compressedData = huffman.compress(data);
            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open output file: " + outputPath.string());
            }
            output.write(reinterpret_cast<const char *>(&fileSize), sizeof(fileSize));
            output.write(reinterpret_cast<const char *>(compressedData.data()), compressedData.size());
            output.close();
        }
        void decompress(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override
        {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open())
            {
                throw std::runtime_error("Failed to open input file: " + inputPath.string());
            }
            size_t originalSize;
            input.read(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));
            std::vector<uint8_t> compressedData;
            uint8_t byte;
            while (input.read(reinterpret_cast<char *>(&byte), 1))
            {
                compressedData.push_back(byte);
            }
            input.close();
            Huffman huffman;
            std::vector<uint8_t> decompressedData = huffman.decompress(compressedData, originalSize);
            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open output file: " + outputPath.string());
            }
            output.write(reinterpret_cast<const char *>(decompressedData.data()), decompressedData.size());
            output.close();
        }
        CompressionType getType() const override
        {
            return CompressionType::Huffman;
        }
        std::string getName() const override
        {
            return "Huffman";
        }
    };
    class Lz77Compression : public Compression
    {
    public:
        void compress(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override
        {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open())
            {
                throw std::runtime_error("Failed to open input file: " + inputPath.string());
            }
            input.seekg(0, std::ios::end);
            size_t fileSize = input.tellg();
            input.seekg(0, std::ios::beg);
            std::vector<uint8_t> data(fileSize);
            input.read(reinterpret_cast<char *>(data.data()), fileSize);
            input.close();
            LZ77 lz77;
            std::vector<uint8_t> compressedData = lz77.compress(data);
            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open output file: " + outputPath.string());
            }
            output.write(reinterpret_cast<const char *>(&fileSize), sizeof(fileSize));
            output.write(reinterpret_cast<const char *>(compressedData.data()), compressedData.size());
            output.close();
        }
        void decompress(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override
        {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open())
            {
                throw std::runtime_error("Failed to open input file: " + inputPath.string());
            }
            size_t originalSize;
            input.read(reinterpret_cast<char *>(&originalSize), sizeof(originalSize));
            input.seekg(0, std::ios::end);
            std::streampos fileEnd = input.tellg();
            input.seekg(sizeof(originalSize), std::ios::beg);
            std::streampos compressedDataStart = input.tellg();
            size_t compressedSize = fileEnd - compressedDataStart;
            std::vector<uint8_t> compressedData(compressedSize);
            input.read(reinterpret_cast<char *>(compressedData.data()), compressedSize);
            input.close();
            LZ77 lz77;
            std::vector<uint8_t> decompressedData = lz77.decompress(compressedData, originalSize);
            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open output file: " + outputPath.string());
            }
            output.write(reinterpret_cast<const char *>(decompressedData.data()), decompressedData.size());
            output.close();
        }
        CompressionType getType() const override
        {
            return CompressionType::Lz77;
        }
        std::string getName() const override
        {
            return "Lz77";
        }
    };
    std::unique_ptr<Compression> createCompressor(CompressionType type)
    {
        switch (type)
        {
        case CompressionType::Huffman:
            return std::make_unique<HuffmanCompression>();
        case CompressionType::Lz77:
            return std::make_unique<Lz77Compression>();
        default:
            throw std::invalid_argument("Invalid compression type");
        }
    }
} 