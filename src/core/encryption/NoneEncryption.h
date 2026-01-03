#pragma once
#include "Encryption.h"

namespace backup::core::encryption
{
    // 空加密器，用于表示未启用加密
    class NoneEncryption : public Encryption
    {
    public:
        NoneEncryption() = default;
        ~NoneEncryption() override = default;

        void setKey(const std::string &key) override;
        void encrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override;
        void decrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override;
        EncryptionType getType() const override;
        std::string getName() const override;
    };
}