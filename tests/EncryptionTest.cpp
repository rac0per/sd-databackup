#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "encryption/Encryption.h"
using namespace backup::core::encryption;
using namespace std::filesystem;
class EncryptionTest : public ::testing::Test {
protected:
    path testRoot;
    path inputFile;
    path encryptedFile;
    path decryptedFile;
    std::string testKey = "test_password_123";
    std::string testContent = "This is a test content for encryption and decryption.";
    void SetUp() override {
        testRoot = temp_directory_path() / "encryption_test";
        create_directories(testRoot);
        inputFile = testRoot / "input.txt";
        encryptedFile = testRoot / "encrypted.bin";
        decryptedFile = testRoot / "decrypted.txt";
        std::ofstream(inputFile) << testContent;
    }
    void TearDown() override {
        remove_all(testRoot);
    }
};
TEST_F(EncryptionTest, MD5Encryption) {
    auto encryptor = createEncryptor(EncryptionType::MD5);
    ASSERT_NE(encryptor, nullptr);
    encryptor->setKey(testKey);
    encryptor->encrypt(inputFile, encryptedFile);
    EXPECT_TRUE(exists(encryptedFile));
    encryptor->decrypt(encryptedFile, decryptedFile);
    EXPECT_TRUE(exists(decryptedFile));
    std::string decryptedContent;
    std::ifstream fileStream(decryptedFile);
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    decryptedContent = buffer.str();
    EXPECT_EQ(decryptedContent, testContent);
}
TEST_F(EncryptionTest, EmptyFileEncryption) {
    auto encryptor = createEncryptor(EncryptionType::MD5);
    ASSERT_NE(encryptor, nullptr);
    encryptor->setKey(testKey);
    path emptyInputFile = testRoot / "empty_input.txt";
    path emptyEncryptedFile = testRoot / "empty_encrypted.bin";
    path emptyDecryptedFile = testRoot / "empty_decrypted.txt";
    std::ofstream(emptyInputFile).close();
    encryptor->encrypt(emptyInputFile, emptyEncryptedFile);
    EXPECT_TRUE(exists(emptyEncryptedFile));
    encryptor->decrypt(emptyEncryptedFile, emptyDecryptedFile);
    EXPECT_TRUE(exists(emptyDecryptedFile));
    EXPECT_TRUE(is_empty(emptyDecryptedFile));
}
TEST_F(EncryptionTest, GetTypeAndName) {
    auto md5Encryptor = createEncryptor(EncryptionType::MD5);
    EXPECT_EQ(md5Encryptor->getType(), EncryptionType::MD5);
    EXPECT_EQ(md5Encryptor->getName(), "MD5");
}