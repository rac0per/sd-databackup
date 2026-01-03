#pragma once
#include <string>
#include <filesystem>
namespace backup::core::encryption {
enum class EncryptionType {
    MD5
};
class Encryption {
public:
    virtual ~Encryption() = default;
    virtual void setKey(const std::string& key) = 0;
    virtual void encrypt(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath) = 0;
    virtual void decrypt(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath) = 0;
    virtual EncryptionType getType() const = 0;
    virtual std::string getName() const = 0;
};
std::unique_ptr<Encryption> createEncryptor(EncryptionType type);
} 