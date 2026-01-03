#pragma once
#include <string>
#include <filesystem>
namespace backup::core::compression {
enum class CompressionType {
    Huffman,
    Lz77
};
class Compression {
public:
    virtual ~Compression() = default;
    virtual void compress(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath) = 0;
    virtual void decompress(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath) = 0;
    virtual CompressionType getType() const = 0;
    virtual std::string getName() const = 0;
};
std::unique_ptr<Compression> createCompressor(CompressionType type);
} 