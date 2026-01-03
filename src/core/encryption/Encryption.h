#pragma once
#include <string>
#include <filesystem>
#include <memory>

namespace backup::core::encryption
{
    // 加密算法类型枚举
    enum class EncryptionType
    {
        None,
        AES, // AES加密算法
    };

    // 加密器抽象基类
    class Encryption
    {
    public:
        virtual ~Encryption() = default;

        // 设置加密密钥
        virtual void setKey(const std::string &key) = 0;

        // 加密文件
        virtual void encrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) = 0;

        // 解密文件
        virtual void decrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) = 0;

        // 获取加密类型
        virtual EncryptionType getType() const = 0;

        // 获取加密名称
        virtual std::string getName() const = 0;
    };

    // 创建加密器工厂函数
    std::unique_ptr<Encryption> createEncryptor(EncryptionType type);
}