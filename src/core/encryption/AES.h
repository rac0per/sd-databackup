#pragma once
#include "Encryption.h"
#include <openssl/evp.h>

namespace backup::core::encryption
{
    class AESEncryption : public Encryption
    {
    public:
        AESEncryption() = default;
        ~AESEncryption() = default;

        void setKey(const std::string &key) override;

        void encrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override;

        void decrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override;

        EncryptionType getType() const override;

        std::string getName() const override;

    private:
        std::string m_key; // 加密密钥

        void deriveKey(const std::string &password, uint8_t *key, uint8_t *iv);
    };
}