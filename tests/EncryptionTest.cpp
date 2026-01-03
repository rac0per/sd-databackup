#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "encryption/Encryption.h"

using namespace backup::core::encryption;
namespace fs = std::filesystem;

class EncryptionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        tmp_dir = fs::temp_directory_path() / "enc_test";
        fs::create_directories(tmp_dir);

        test_input = tmp_dir / "in.txt";
        std::ofstream(test_input) << "This is a test file for AES encryption and decryption.";

        test_password = "test_password_123";
        wrong_password = "wrong_password_456";
    }

    void TearDown() override
    {
        fs::remove_all(tmp_dir);
    }

    fs::path tmp_dir;
    fs::path test_input;
    std::string test_password;
    std::string wrong_password;
};

// 测试None加密器
TEST_F(EncryptionTest, NoneEncryptorTest)
{
    auto enc = createEncryptor(EncryptionType::None);

    EXPECT_EQ(enc->getType(), EncryptionType::None);
    EXPECT_EQ(enc->getName(), "None");

    fs::path out = tmp_dir / "out.txt";

    EXPECT_THROW(enc->encrypt(test_input, out), std::runtime_error);
    EXPECT_THROW(enc->decrypt(test_input, out), std::runtime_error);
}

// 测试AES加密器创建
TEST_F(EncryptionTest, AesEncryptorCreation)
{
    auto enc = createEncryptor(EncryptionType::AES);

    EXPECT_EQ(enc->getType(), EncryptionType::AES);
    EXPECT_EQ(enc->getName(), "AES");
}

// 测试AES加密解密功能
TEST_F(EncryptionTest, AesEncryptDecrypt)
{
    auto enc = createEncryptor(EncryptionType::AES);
    enc->setKey(test_password);

    fs::path encrypted = tmp_dir / "encrypted.bin";
    fs::path decrypted = tmp_dir / "decrypted.txt";

    enc->encrypt(test_input, encrypted);
    EXPECT_TRUE(fs::exists(encrypted));

    enc->decrypt(encrypted, decrypted);
    EXPECT_TRUE(fs::exists(decrypted));

    std::ifstream original(test_input);
    std::ifstream decrypted_file(decrypted);
    std::string original_content((std::istreambuf_iterator<char>(original)), std::istreambuf_iterator<char>());
    std::string decrypted_content((std::istreambuf_iterator<char>(decrypted_file)), std::istreambuf_iterator<char>());

    EXPECT_EQ(original_content, decrypted_content);
}

// 测试AES空密码
TEST_F(EncryptionTest, AesEmptyPassword)
{
    auto enc = createEncryptor(EncryptionType::AES);
    enc->setKey("");

    fs::path out = tmp_dir / "out.txt";

    EXPECT_THROW(enc->encrypt(test_input, out), std::runtime_error);
}

// 测试AES错误密码解密
TEST_F(EncryptionTest, AesWrongPassword)
{
    auto enc = createEncryptor(EncryptionType::AES);
    enc->setKey(test_password);

    fs::path encrypted = tmp_dir / "encrypted.bin";
    fs::path decrypted = tmp_dir / "decrypted.txt";

    enc->encrypt(test_input, encrypted);

    auto enc_wrong = createEncryptor(EncryptionType::AES);
    enc_wrong->setKey(wrong_password);

    EXPECT_THROW(enc_wrong->decrypt(encrypted, decrypted), std::runtime_error);
}

// 测试AES加密器重用
TEST_F(EncryptionTest, AesEncryptorReuse)
{
    auto enc = createEncryptor(EncryptionType::AES);
    enc->setKey(test_password);

    fs::path encrypted1 = tmp_dir / "encrypted1.bin";
    fs::path encrypted2 = tmp_dir / "encrypted2.bin";
    fs::path decrypted1 = tmp_dir / "decrypted1.txt";
    fs::path decrypted2 = tmp_dir / "decrypted2.txt";

    enc->encrypt(test_input, encrypted1);
    enc->decrypt(encrypted1, decrypted1);

    enc->encrypt(test_input, encrypted2);
    enc->decrypt(encrypted2, decrypted2);

    std::ifstream decrypted1_file(decrypted1);
    std::ifstream decrypted2_file(decrypted2);
    std::string decrypted1_content((std::istreambuf_iterator<char>(decrypted1_file)), std::istreambuf_iterator<char>());
    std::string decrypted2_content((std::istreambuf_iterator<char>(decrypted2_file)), std::istreambuf_iterator<char>());

    EXPECT_EQ(decrypted1_content, decrypted2_content);
}