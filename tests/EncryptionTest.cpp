#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "encryption/Encryption.h"

using namespace backup::core::encryption;
namespace fs = std::filesystem;

TEST(EncryptionDisabled, NoAlgorithmsImplemented) {
    auto enc = createEncryptor(EncryptionType::None);
    EXPECT_EQ(enc->getType(), EncryptionType::None);
    EXPECT_EQ(enc->getName(), "None");

    fs::path tmp = fs::temp_directory_path() / "enc_none";
    fs::create_directories(tmp);
    fs::path in = tmp / "in.txt";
    fs::path out = tmp / "out.txt";
    std::ofstream(in) << "data";

    EXPECT_THROW(enc->encrypt(in, out), std::runtime_error);
    EXPECT_THROW(enc->decrypt(in, out), std::runtime_error);

    fs::remove_all(tmp);
}