#include "Encryption.h"
#include <stdexcept>

namespace backup::core::encryption
{
    // 目前未提供任何加密算法，保留接口占位
    class NoopEncryption : public Encryption
    {
    public:
        void setKey(const std::string &) override {}
        void encrypt(const std::filesystem::path &, const std::filesystem::path &) override
        {
            throw std::runtime_error("Encryption not supported");
        }
        void decrypt(const std::filesystem::path &, const std::filesystem::path &) override
        {
            throw std::runtime_error("Encryption not supported");
        }
        EncryptionType getType() const override { return EncryptionType::None; }
        std::string getName() const override { return "None"; }
    };

    std::unique_ptr<Encryption> createEncryptor(EncryptionType type)
    {
        if (type == EncryptionType::None)
        {
            return std::make_unique<NoopEncryption>();
        }
        throw std::invalid_argument("Invalid encryption type");
    }
} 