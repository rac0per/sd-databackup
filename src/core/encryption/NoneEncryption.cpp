#include "NoneEncryption.h"
#include <fstream>
#include <stdexcept>

namespace backup::core::encryption
{
    void NoneEncryption::setKey(const std::string &key)
    {
        // 空加密器不需要密钥，忽略
    }

    void NoneEncryption::encrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath)
    {
        throw std::runtime_error("None encryption type does not support encrypt operation");
    }

    void NoneEncryption::decrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath)
    {
        throw std::runtime_error("None encryption type does not support decrypt operation");
    }

    EncryptionType NoneEncryption::getType() const
    {
        return EncryptionType::None;
    }

    std::string NoneEncryption::getName() const
    {
        return "None";
    }
}