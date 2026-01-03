#include "AES.h"
#include <fstream>
#include <stdexcept>
#include <vector>
#include <openssl/evp.h>

namespace backup::core::encryption
{

    void AESEncryption::setKey(const std::string &key)
    {
        m_key = key;
    }

    EncryptionType AESEncryption::getType() const
    {
        return EncryptionType::AES;
    }

    std::string AESEncryption::getName() const
    {
        return "AES";
    }

    void AESEncryption::deriveKey(const std::string &password, uint8_t *key, uint8_t *iv)
    {
        uint8_t hash[32]; // SHA256产出32字节哈希

        EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
        const EVP_MD *md = EVP_sha256();

        EVP_DigestInit_ex(md_ctx, md, nullptr);
        EVP_DigestUpdate(md_ctx, password.c_str(), password.size());
        EVP_DigestFinal_ex(md_ctx, hash, nullptr);

        EVP_MD_CTX_free(md_ctx);

        // AES-256需要32字节密钥，CBC模式需要16字节IV
        memcpy(key, hash, 32);
        memcpy(iv, hash, 16); // 使用哈希前16字节作为IV
    }

    void AESEncryption::encrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath)
    {
        if (m_key.empty())
        {
            throw std::runtime_error("Encryption key not set");
        }

        std::ifstream input(inputPath, std::ios::binary);
        if (!input.is_open())
        {
            throw std::runtime_error("Failed to open input file: " + inputPath.string());
        }

        std::ofstream output(outputPath, std::ios::binary);
        if (!output.is_open())
        {
            throw std::runtime_error("Failed to open output file: " + outputPath.string());
        }

        uint8_t key[32], iv[16];
        deriveKey(m_key, key, iv);

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP context");
        }

        const EVP_CIPHER *cipher = EVP_aes_256_cbc();
        if (!EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }

        const size_t BUFFER_SIZE = 4096;
        std::vector<uint8_t> inBuffer(BUFFER_SIZE);
        std::vector<uint8_t> outBuffer(BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH);
        int inLen, outLen;

        while (input.good())
        {
            input.read(reinterpret_cast<char *>(inBuffer.data()), BUFFER_SIZE);
            inLen = input.gcount();
            if (inLen <= 0)
                break;

            if (!EVP_EncryptUpdate(ctx, outBuffer.data(), &outLen, inBuffer.data(), inLen))
            {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Encryption failed");
            }

            output.write(reinterpret_cast<const char *>(outBuffer.data()), outLen);
        }

        if (!EVP_EncryptFinal_ex(ctx, outBuffer.data(), &outLen))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Encryption finalization failed");
        }

        output.write(reinterpret_cast<const char *>(outBuffer.data()), outLen);

        EVP_CIPHER_CTX_free(ctx);
    }

    void AESEncryption::decrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath)
    {
        if (m_key.empty())
        {
            throw std::runtime_error("Encryption key not set");
        }

        std::ifstream input(inputPath, std::ios::binary);
        if (!input.is_open())
        {
            throw std::runtime_error("Failed to open input file: " + inputPath.string());
        }

        std::ofstream output(outputPath, std::ios::binary);
        if (!output.is_open())
        {
            throw std::runtime_error("Failed to open output file: " + outputPath.string());
        }

        uint8_t key[32], iv[16];
        deriveKey(m_key, key, iv);

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("Failed to create EVP context");
        }

        const EVP_CIPHER *cipher = EVP_aes_256_cbc();
        if (!EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }

        const size_t BUFFER_SIZE = 4096;
        std::vector<uint8_t> inBuffer(BUFFER_SIZE);
        std::vector<uint8_t> outBuffer(BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH);
        int inLen, outLen;

        while (input.good())
        {
            input.read(reinterpret_cast<char *>(inBuffer.data()), BUFFER_SIZE);
            inLen = input.gcount();
            if (inLen <= 0)
                break;

            if (!EVP_DecryptUpdate(ctx, outBuffer.data(), &outLen, inBuffer.data(), inLen))
            {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Decryption failed");
            }

            output.write(reinterpret_cast<const char *>(outBuffer.data()), outLen);
        }

        if (!EVP_DecryptFinal_ex(ctx, outBuffer.data(), &outLen))
        {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Decryption finalization failed");
        }

        output.write(reinterpret_cast<const char *>(outBuffer.data()), outLen);

        EVP_CIPHER_CTX_free(ctx);
    }
}