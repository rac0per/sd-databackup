#include "Encryption.h"
#include "AES.h"
#include "NoneEncryption.h"

namespace backup::core::encryption
{
    // 创建加密器工厂函数
    std::unique_ptr<Encryption> createEncryptor(EncryptionType type)
    {
        switch (type)
        {
        case EncryptionType::None:
            return std::make_unique<NoneEncryption>();
        case EncryptionType::AES:
            return std::make_unique<AESEncryption>();
        default:
            throw std::invalid_argument("Invalid encryption type: " + std::to_string(static_cast<int>(type)));
        }
    }
}