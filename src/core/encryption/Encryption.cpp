#include "Encryption.h"
#include "MD5.h"
#include <fstream>
#include <stdexcept>
#include <vector>
namespace backup::core::encryption
{
    class MD5Encryption : public Encryption
    {
    public:
        void setKey(const std::string &key) override
        {
        }
        void encrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override
        {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open())
            {
                throw std::runtime_error("Failed to open input file: " + inputPath.string());
            }
            input.seekg(0, std::ios::end);
            size_t fileSize = input.tellg();
            input.seekg(0, std::ios::beg);
            std::vector<uint8_t> buffer(fileSize);
            input.read(reinterpret_cast<char *>(buffer.data()), fileSize);
            input.close();
            MD5 md5;
            md5.update(buffer.data(), buffer.size());
            std::string hash = md5.final();
            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open output file: " + outputPath.string());
            }
            output.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
            output.write(hash.c_str(), hash.size());
            output.close();
        }
        void decrypt(const std::filesystem::path &inputPath, const std::filesystem::path &outputPath) override
        {
            std::ifstream input(inputPath, std::ios::binary);
            if (!input.is_open())
            {
                throw std::runtime_error("Failed to open input file: " + inputPath.string());
            }
            input.seekg(0, std::ios::end);
            size_t fileSize = input.tellg();
            input.seekg(0, std::ios::beg);
            size_t contentSize = (fileSize >= 32) ? (fileSize - 32) : 0;
            std::vector<uint8_t> buffer(contentSize);
            input.read(reinterpret_cast<char *>(buffer.data()), contentSize);
            input.close();
            std::ofstream output(outputPath, std::ios::binary);
            if (!output.is_open())
            {
                throw std::runtime_error("Failed to open output file: " + outputPath.string());
            }
            output.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
            output.close();
        }
        EncryptionType getType() const override
        {
            return EncryptionType::MD5;
        }
        std::string getName() const override
        {
            return "MD5";
        }
    };
    std::unique_ptr<Encryption> createEncryptor(EncryptionType type)
    {
        switch (type)
        {
        case EncryptionType::MD5:
            return std::make_unique<MD5Encryption>();
        default:
            throw std::invalid_argument("Invalid encryption type");
        }
    }
} 