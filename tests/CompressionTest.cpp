#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include "compression/Compression.h"

using namespace backup::core::compression;
namespace fs = std::filesystem;

class CompressionTest : public ::testing::Test {
protected:
    fs::path root;
    fs::path inputFile;
    fs::path compressedFile;
    fs::path decompressedFile;
    std::string textPayload;

    void SetUp() override {
        root = fs::temp_directory_path() / "compression_suite";
        fs::create_directories(root);
        inputFile = root / "in.bin";
        compressedFile = root / "out.bin";
        decompressedFile = root / "out.txt";
        textPayload = std::string(4096, 'A');
        std::ofstream(inputFile, std::ios::binary) << textPayload;
    }

    void TearDown() override {
        fs::remove_all(root);
    }
};

TEST_F(CompressionTest, HuffmanRoundTripText) {
    auto c = createCompressor(CompressionType::Huffman);
    c->compress(inputFile, compressedFile);
    c->decompress(compressedFile, decompressedFile);
    std::ifstream in(decompressedFile, std::ios::binary);
    std::string restored((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_EQ(restored, textPayload);
}

TEST_F(CompressionTest, Lz77RoundTripText) {
    auto c = createCompressor(CompressionType::Lz77);
    c->compress(inputFile, compressedFile);
    c->decompress(compressedFile, decompressedFile);
    std::ifstream in(decompressedFile, std::ios::binary);
    std::string restored((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_EQ(restored, textPayload);
}

TEST_F(CompressionTest, EmptyFile) {
    fs::path emptyIn = root / "empty";
    fs::path emptyComp = root / "empty.bin";
    fs::path emptyOut = root / "empty.out";
    std::ofstream(emptyIn).close();

    auto c = createCompressor(CompressionType::Huffman);
    c->compress(emptyIn, emptyComp);
    c->decompress(emptyComp, emptyOut);

    EXPECT_TRUE(fs::exists(emptyOut));
    EXPECT_TRUE(fs::is_empty(emptyOut));
}

TEST_F(CompressionTest, BinaryPayloadWithLz77) {
    std::vector<uint8_t> data(1024);
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> dist(0, 255);
    for (auto& b : data) b = static_cast<uint8_t>(dist(rng));

    std::ofstream(inputFile, std::ios::binary).write(reinterpret_cast<const char*>(data.data()), data.size());

    auto c = createCompressor(CompressionType::Lz77);
    c->compress(inputFile, compressedFile);
    c->decompress(compressedFile, decompressedFile);

    std::vector<uint8_t> restored(data.size());
    std::ifstream out(decompressedFile, std::ios::binary);
    out.read(reinterpret_cast<char*>(restored.data()), restored.size());
    EXPECT_EQ(out.gcount(), static_cast<std::streamsize>(restored.size()));
    EXPECT_EQ(restored, data);
}

TEST_F(CompressionTest, TypeAndNameExposeAlgorithm) {
    auto h = createCompressor(CompressionType::Huffman);
    auto l = createCompressor(CompressionType::Lz77);
    EXPECT_EQ(h->getType(), CompressionType::Huffman);
    EXPECT_EQ(h->getName(), "Huffman");
    EXPECT_EQ(l->getType(), CompressionType::Lz77);
    EXPECT_EQ(l->getName(), "Lz77");
}