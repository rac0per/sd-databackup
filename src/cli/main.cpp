#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "compression/Compression.h"
#include "encryption/Encryption.h"
#include "backup/BackupManager.h"
#include "backup/BackupMetadata.h"
#include "filesystem/FileTree.h"

using namespace backup::core::compression;
using namespace backup::core::encryption;
using namespace backup::core;
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    // 简易封装：将目录打包成单文件（供压缩使用）
    const auto packDirectory = [](const fs::path &srcDir, const fs::path &packFile) -> bool {
        if (!fs::exists(srcDir) || !fs::is_directory(srcDir))
            return false;

        struct Entry
        {
            bool isDir;
            fs::path rel;
            std::uint64_t size;
        };

        std::vector<Entry> entries;
        const auto root = fs::absolute(srcDir);
        for (auto it = fs::recursive_directory_iterator(root); it != fs::recursive_directory_iterator(); ++it)
        {
            const auto rel = fs::relative(it->path(), root);
            if (rel.empty())
                continue;
            if (it->is_directory())
            {
                entries.push_back({true, rel, 0});
            }
            else if (it->is_regular_file())
            {
                entries.push_back({false, rel, static_cast<std::uint64_t>(fs::file_size(it->path()))});
            }
        }

        std::ofstream out(packFile, std::ios::binary | std::ios::trunc);
        if (!out.is_open())
            return false;

        const std::uint32_t magic = 0x5344504b; // 'SDPK'
        const std::uint32_t version = 1;
        out.write(reinterpret_cast<const char *>(&magic), sizeof(magic));
        out.write(reinterpret_cast<const char *>(&version), sizeof(version));
        const std::uint32_t count = static_cast<std::uint32_t>(entries.size());
        out.write(reinterpret_cast<const char *>(&count), sizeof(count));

        std::vector<char> buffer(64 * 1024);
        for (const auto &e : entries)
        {
            const std::string relStr = e.rel.generic_string();
            const std::uint32_t pathLen = static_cast<std::uint32_t>(relStr.size());
            const std::uint8_t type = e.isDir ? 0u : 1u;

            out.write(reinterpret_cast<const char *>(&type), sizeof(type));
            out.write(reinterpret_cast<const char *>(&pathLen), sizeof(pathLen));
            out.write(reinterpret_cast<const char *>(&e.size), sizeof(e.size));
            out.write(relStr.data(), pathLen);

            if (!e.isDir)
            {
                std::ifstream in(root / e.rel, std::ios::binary);
                if (!in.is_open())
                    return false;
                std::uint64_t remaining = e.size;
                while (remaining > 0)
                {
                    const auto chunk = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), remaining));
                    in.read(buffer.data(), chunk);
                    out.write(buffer.data(), chunk);
                    remaining -= chunk;
                }
            }
        }

        return out.good();
    };

    // 检查文件是否为打包格式
    const auto isPackageFile = [](const fs::path &p) -> bool {
        std::ifstream in(p, std::ios::binary);
        if (!in.is_open())
            return false;
        std::uint32_t magic = 0;
        in.read(reinterpret_cast<char *>(&magic), sizeof(magic));
        return magic == 0x5344504b; // 'SDPK'
    };

    // 解包到指定目录
    const auto unpackToDirectory = [](const fs::path &packFile, const fs::path &destDir) -> bool {
        std::ifstream in(packFile, std::ios::binary);
        if (!in.is_open())
            return false;

        std::uint32_t magic = 0, version = 0, count = 0;
        in.read(reinterpret_cast<char *>(&magic), sizeof(magic));
        in.read(reinterpret_cast<char *>(&version), sizeof(version));
        in.read(reinterpret_cast<char *>(&count), sizeof(count));
        if (magic != 0x5344504b || version != 1)
            return false;

        std::vector<char> buffer(64 * 1024);
        for (std::uint32_t i = 0; i < count; ++i)
        {
            std::uint8_t type = 0;
            std::uint32_t pathLen = 0;
            std::uint64_t size = 0;
            in.read(reinterpret_cast<char *>(&type), sizeof(type));
            in.read(reinterpret_cast<char *>(&pathLen), sizeof(pathLen));
            in.read(reinterpret_cast<char *>(&size), sizeof(size));
            std::string rel(pathLen, '\0');
            in.read(rel.data(), pathLen);
            fs::path outPath = destDir / rel;

            if (type == 0)
            {
                fs::create_directories(outPath);
            }
            else
            {
                fs::create_directories(outPath.parent_path());
                std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
                if (!out.is_open())
                    return false;
                std::uint64_t remaining = size;
                while (remaining > 0)
                {
                    const auto chunk = static_cast<std::size_t>(std::min<std::uint64_t>(buffer.size(), remaining));
                    in.read(buffer.data(), chunk);
                    out.write(buffer.data(), chunk);
                    remaining -= chunk;
                }
            }
        }

        return true;
    };

    if (argc < 4)
    {
        std::cerr << "用法: " << argv[0] << " <命令> <参数...>\n";
        std::cerr << "  命令列表:\n";
        std::cerr << "    1. compress <输入路径> <输出文件> <算法> [-W <密码>]      压缩文件或目录（目录会先打包）\n";
        std::cerr << "      算法: huffman | lz77\n";
        std::cerr << "      -W <密码>: 启用AES加密并设置密码\n";
        std::cerr << "    2. decompress <输入文件> <输出路径> <算法> [-W <密码>]    解压文件；若包含目录包则解包到输出路径\n";
        std::cerr << "      算法: huffman | lz77\n";
        std::cerr << "      -W <密码>: 启用AES解密并设置密码\n";
        std::cerr << "    3. backup <源目录> <备份目录> [mirror] [compress=<算法>] [-W <密码>]         备份目录树\n";
        std::cerr << "      mirror: 镜像模式，删除目标目录中不存在的文件\n";
        std::cerr << "      compress=<算法>: 设置压缩算法 (huffman | lz77 | none)\n";
        std::cerr << "      -W <密码>: 启用AES加密并设置密码\n";
        std::cerr << "    4. restore <备份目录> <还原目录> [-W <密码>]             从备份还原目录树\n";
        std::cerr << "      -W <密码>: 设置AES解密密码\n";
        return 1;
    }

    std::string command = argv[1];

    try
    {
        if (command == "compress" || command == "decompress")
        {
            // 解析参数
            std::string inputPath = argv[2];
            std::string outputPath = argv[3];
            std::string algorithm = argv[4];
            std::string password;
            bool enableEncryption = false;
            const bool inputIsDir = fs::exists(inputPath) && fs::is_directory(inputPath);

            // 解析可选的-W参数
            for (int i = 5; i < argc; ++i)
            {
                std::string arg = argv[i];
                if ((arg == "-W" || arg == "-w") && i + 1 < argc)
                {
                    password = argv[++i];
                    enableEncryption = true;
                }
                else
                {
                    std::cerr << "用法: " << argv[0] << " " << command << " <输入文件> <输出文件> <算法> [-W <密码>]\n";
                    std::cerr << "  算法: huffman | lz77\n";
                    return 1;
                }
            }

            if (command == "compress")
            {
                fs::path tempPath = outputPath + ".tmp";

                // 若输入为目录，先打包
                fs::path packedInput = inputPath;
                fs::path tempPackPath;
                if (inputIsDir)
                {
                    tempPackPath = outputPath + ".packtmp";
                    if (!packDirectory(inputPath, tempPackPath))
                    {
                        std::cerr << "打包目录失败: " << inputPath << std::endl;
                        return 1;
                    }
                    packedInput = tempPackPath;
                }

                // 先压缩
                if (algorithm == "huffman")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Huffman);
                    compressor->compress(packedInput, tempPath);
                }
                else if (algorithm == "lz77")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Lz77);
                    compressor->compress(packedInput, tempPath);
                }
                else
                {
                    std::cerr << "不支持的压缩算法: " << algorithm << std::endl;
                    if (inputIsDir && fs::exists(tempPackPath))
                        fs::remove(tempPackPath);
                    return 1;
                }

                // 再加密（如果启用）
                if (enableEncryption)
                {
                    auto encryptor = createEncryptor(EncryptionType::AES);
                    encryptor->setKey(password);
                    encryptor->encrypt(tempPath, outputPath);
                    fs::remove(tempPath);
                    if (inputIsDir)
                        std::cout << "使用" << algorithm << "算法压缩并使用AES加密目录成功！\n";
                    else
                        std::cout << "使用" << algorithm << "算法压缩并使用AES加密文件成功！\n";
                }
                else
                {
                    fs::rename(tempPath, outputPath);
                    if (inputIsDir)
                        std::cout << "使用" << algorithm << "算法压缩目录成功！\n";
                    else
                        std::cout << "使用" << algorithm << "算法压缩文件成功！\n";
                }

                if (inputIsDir && fs::exists(tempPackPath))
                    fs::remove(tempPackPath);
            }
            else if (command == "decompress")
            {
                fs::path tempPath = outputPath + ".tmp";      // 解密后的临时文件
                fs::path tempDecompressed = outputPath + ".tmp.decomp"; // 解压后的临时文件

                // 先解密（如果需要）
                if (enableEncryption)
                {
                    auto encryptor = createEncryptor(EncryptionType::AES);
                    encryptor->setKey(password);
                    encryptor->decrypt(inputPath, tempPath);
                }
                else
                {
                    fs::copy_file(inputPath, tempPath, fs::copy_options::overwrite_existing);
                }

                // 再解压
                if (algorithm == "huffman")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Huffman);
                    compressor->decompress(tempPath, tempDecompressed);
                }
                else if (algorithm == "lz77")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Lz77);
                    compressor->decompress(tempPath, tempDecompressed);
                }
                else
                {
                    std::cerr << "不支持的解压算法: " << algorithm << std::endl;
                    fs::remove(tempPath);
                    return 1;
                }

                fs::remove(tempPath);

                // 如果解压结果是目录打包文件，进一步解包到目标目录
                if (isPackageFile(tempDecompressed))
                {
                    fs::path destDir = outputPath;
                    if (fs::exists(destDir) && !fs::is_directory(destDir))
                    {
                        std::cerr << "输出路径已存在且不是目录: " << destDir << std::endl;
                        fs::remove(tempDecompressed);
                        return 1;
                    }
                    fs::create_directories(destDir);
                    if (!unpackToDirectory(tempDecompressed, destDir))
                    {
                        std::cerr << "解包目录失败" << std::endl;
                        fs::remove(tempDecompressed);
                        return 1;
                    }
                    fs::remove(tempDecompressed);
                    std::cout << "使用" << algorithm << "算法解压" << (enableEncryption ? "并解密" : "") << "目录成功！\n";
                }
                else
                {
                    if (fs::exists(outputPath))
                        fs::remove(outputPath);
                    fs::rename(tempDecompressed, outputPath);
                    std::cout << "使用" << algorithm << "算法解压" << (enableEncryption ? "并解密" : "") << "文件成功！\n";
                }
            }
        }
        else if (command == "backup")
        {
            if (argc < 4)
            {
                std::cerr << "用法: " << argv[0] << " backup <源目录> <备份目录> [mirror] [compress=<算法>]\n";
                std::cerr << "  算法: none | huffman | lz77\n";
                return 1;
            }

            std::string sourceDir = argv[2];
            std::string backupDir = argv[3];
            bool mirrorMode = false;
            bool enableCompression = false;
            BackupManager::CompressionType compressionType = BackupManager::CompressionType::None;
            bool enableEncryption = false;
            std::string encryptionKey;

            // 解析可选参数
            for (int i = 4; i < argc; ++i)
            {
                std::string arg = argv[i];
                if (arg == "mirror")
                {
                    mirrorMode = true;
                }
                else if (arg.find("compress=") == 0)
                {
                    std::string algo = arg.substr(9);
                    if (algo == "huffman")
                    {
                        enableCompression = true;
                        compressionType = BackupManager::CompressionType::Huffman;
                    }
                    else if (algo == "lz77")
                    {
                        enableCompression = true;
                        compressionType = BackupManager::CompressionType::Lz77;
                    }
                    else if (algo == "none")
                    {
                        enableCompression = false;
                        compressionType = BackupManager::CompressionType::None;
                    }
                    else
                    {
                        std::cerr << "不支持的压缩算法: " << algo << std::endl;
                        return 1;
                    }
                }
                else if ((arg == "-W" || arg == "-w") && i + 1 < argc)
                {
                    encryptionKey = argv[++i];
                    enableEncryption = true;
                }
                else
                {
                    std::cerr << "不支持的参数: " << arg << std::endl;
                    return 1;
                }
            }

            // 创建备份配置
            BackupManager::BackupConfig config;
            config.sourceRoot = sourceDir;
            config.backupRoot = backupDir;
            config.deleteRemoved = mirrorMode;
            config.dryRun = false;
            config.enableCompression = enableCompression;
            config.compressionType = compressionType;
            config.encryptionType = BackupManager::EncryptionType::AES;
            config.encryptionKey = encryptionKey;
            config.enableEncryption = enableEncryption;

            // 创建备份管理器并执行备份
            BackupManager manager(config);
            std::cout << "正在扫描目录...\n";
            manager.scan();

            std::cout << "正在生成备份计划...\n";
            auto plan = manager.buildPlan();

            std::cout << "正在执行备份计划...\n";
            manager.executePlan(plan);

            std::cout << "目录备份完成！\n";
        }
        else if (command == "restore")
        {
            std::string backupDir = argv[2];
            std::string restoreDir = argv[3];
            std::string encryptionKey;

            // 解析可选的-W参数
            for (int i = 4; i < argc; ++i)
            {
                std::string arg = argv[i];
                if ((arg == "-W" || arg == "-w") && i + 1 < argc)
                {
                    encryptionKey = argv[++i];
                }
                else
                {
                    std::cerr << "用法: " << argv[0] << " restore <备份目录> <还原目录> [-W <密码>]\n";
                    return 1;
                }
            }

            // 创建备份管理器，配置备份目录
            BackupManager::BackupConfig config;
            config.backupRoot = backupDir;
            config.encryptionKey = encryptionKey;

            // 创建备份管理器并执行还原
            BackupManager manager(config);
            std::cout << "正在从备份还原...\n";
            manager.restore(restoreDir);

            std::cout << "目录还原完成！\n";
        }
        else
        {
            std::cerr << "无效的命令: " << command << std::endl;
            std::cerr << "请使用 'compress', 'decompress', 'backup' 或 'restore'\n";
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "操作失败: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
