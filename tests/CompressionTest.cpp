#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "compression/Compression.h"
using namespace backup::core::compression;
using namespace std::filesystem;
class CompressionTest : public ::testing::Test {
protected:
    path testRoot;
    path inputFile;
    path compressedFile;
    path decompressedFile;
    std::string testContent = "This is a test content for compression and decompression. "
                             "The content is repeated multiple times to make it larger. "
                             "This is a test content for compression and decompression. "
                             "The content is repeated multiple times to make it larger. "
                             "This is a test content for compression and decompression. "
                             "The content is repeated multiple times to make it larger. "
                             "This is a test content for compression and decompression. "
                             "The content is repeated multiple times to make it larger. "
                             "This is a test content for compression and decompression. "
                             "The content is repeated multiple times to make it larger.";
    void SetUp() override {
        testRoot = temp_directory_path() / "compression_test";
        create_directories(testRoot);
        inputFile = testRoot / "input.txt";
        compressedFile = testRoot / "compressed.bin";
        decompressedFile = testRoot / "decompressed.txt";
        std::ofstream(inputFile) << testContent;
    }
    void TearDown() override {
        remove_all(testRoot);
    }
};
TEST_F(CompressionTest, HuffmanCompression) {
    auto compressor = createCompressor(CompressionType::Huffman);
    ASSERT_NE(compressor, nullptr);
    compressor->compress(inputFile, compressedFile);
    EXPECT_TRUE(exists(compressedFile));
    compressor->decompress(compressedFile, decompressedFile);
    EXPECT_TRUE(exists(decompressedFile));
    std::string decompressedContent;
    std::ifstream fileStream(decompressedFile);
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    decompressedContent = buffer.str();
    EXPECT_EQ(decompressedContent, testContent);
}
TEST_F(CompressionTest, Lz77Compression) {
    auto compressor = createCompressor(CompressionType::Lz77);
    ASSERT_NE(compressor, nullptr);
    compressor->compress(inputFile, compressedFile);
    EXPECT_TRUE(exists(compressedFile));
    compressor->decompress(compressedFile, decompressedFile);
    EXPECT_TRUE(exists(decompressedFile));
    std::string decompressedContent;
    std::ifstream fileStream(decompressedFile);
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    decompressedContent = buffer.str();
    EXPECT_EQ(decompressedContent, testContent);
}
TEST_F(CompressionTest, EmptyFileCompression) {
    auto compressor = createCompressor(CompressionType::Huffman);
    ASSERT_NE(compressor, nullptr);
    path emptyInputFile = testRoot / "empty_input.txt";
    path emptyCompressedFile = testRoot / "empty_compressed.bin";
    path emptyDecompressedFile = testRoot / "empty_decompressed.txt";
    std::ofstream(emptyInputFile).close();
    compressor->compress(emptyInputFile, emptyCompressedFile);
    EXPECT_TRUE(exists(emptyCompressedFile));
    compressor->decompress(emptyCompressedFile, emptyDecompressedFile);
    EXPECT_TRUE(exists(emptyDecompressedFile));
    EXPECT_TRUE(is_empty(emptyDecompressedFile));
}
TEST_F(CompressionTest, GetTypeAndName) {
    auto huffmanCompressor = createCompressor(CompressionType::Huffman);
    auto lz77Compressor = createCompressor(CompressionType::Lz77);
    EXPECT_EQ(huffmanCompressor->getType(), CompressionType::Huffman);
    EXPECT_EQ(huffmanCompressor->getName(), "Huffman");
    EXPECT_EQ(lz77Compressor->getType(), CompressionType::Lz77);
    EXPECT_EQ(lz77Compressor->getName(), "Lz77");
}
TEST_F(CompressionTest, CompressedFileExists) {
    auto compressor = createCompressor(CompressionType::Huffman);
    ASSERT_NE(compressor, nullptr);
    EXPECT_FALSE(exists(compressedFile));
    compressor->compress(inputFile, compressedFile);
    EXPECT_TRUE(exists(compressedFile));
}
TEST_F(CompressionTest, DecompressedFileExists) {
    auto compressor = createCompressor(CompressionType::Lz77);
    ASSERT_NE(compressor, nullptr);
    compressor->compress(inputFile, compressedFile);
    EXPECT_FALSE(exists(decompressedFile));
    compressor->decompress(compressedFile, decompressedFile);
    EXPECT_TRUE(exists(decompressedFile));
}