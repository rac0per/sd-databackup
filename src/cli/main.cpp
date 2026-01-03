#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
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
    if (argc < 4)
    {
        std::cerr << "用法: " << argv[0] << " <命令> <参数...>\n";
        std::cerr << "  命令列表:\n";
        std::cerr << "    1. compress <输入文件> <输出文件> <算法>      压缩单个文件\n";
        std::cerr << "      算法: huffman | lz77\n";
        std::cerr << "    2. decompress <输入文件> <输出文件> <算法>    解压单个文件\n";
        std::cerr << "      算法: huffman | lz77\n";
        std::cerr << "    3. backup <源目录> <备份目录> [mirror]         备份目录树\n";
        std::cerr << "      mirror: 镜像模式，删除目标目录中不存在的文件\n";
        std::cerr << "    4. restore <备份目录> <还原目录>             从备份还原目录树\n";
        return 1;
    }

    std::string command = argv[1];

    try
    {
        if (command == "compress" || command == "decompress")
        {
            if (argc < 5)
            {
                std::cerr << "用法: " << argv[0] << " " << command << " <输入文件> <输出文件> <算法>\n";
                    std::cerr << "  算法: huffman | lz77\n";
                return 1;
            }

            std::string inputPath = argv[2];
            std::string outputPath = argv[3];
            std::string algorithm = argv[4];

            if (command == "compress")
            {
                if (algorithm == "huffman")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Huffman);
                    compressor->compress(inputPath, outputPath);
                    std::cout << "使用Huffman算法压缩文件成功！\n";
                }
                else if (algorithm == "lz77")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Lz77);
                    compressor->compress(inputPath, outputPath);
                    std::cout << "使用LZ77算法压缩文件成功！\n";
                }
                else
                {
                    std::cerr << "不支持的压缩/加密算法: " << algorithm << std::endl;
                    return 1;
                }
            }
            else if (command == "decompress")
            {
                if (algorithm == "huffman")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Huffman);
                    compressor->decompress(inputPath, outputPath);
                    std::cout << "使用Huffman算法解压文件成功！\n";
                }
                else if (algorithm == "lz77")
                {
                    auto compressor = createCompressor(backup::core::compression::CompressionType::Lz77);
                    compressor->decompress(inputPath, outputPath);
                    std::cout << "使用LZ77算法解压文件成功！\n";
                }
                else
                {
                    std::cerr << "不支持的解压缩/解密算法: " << algorithm << std::endl;
                    return 1;
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

            // 解析可选参数
            for (int i = 4; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "mirror") {
                    mirrorMode = true;
                } else if (arg.find("compress=") == 0) {
                    std::string algo = arg.substr(9);
                    if (algo == "huffman") {
                        enableCompression = true;
                        compressionType = BackupManager::CompressionType::Huffman;
                    } else if (algo == "lz77") {
                        enableCompression = true;
                        compressionType = BackupManager::CompressionType::Lz77;
                    } else if (algo == "none") {
                        enableCompression = false;
                        compressionType = BackupManager::CompressionType::None;
                    } else {
                        std::cerr << "不支持的压缩算法: " << algo << std::endl;
                        return 1;
                    }
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
            config.encryptionType = BackupManager::EncryptionType::None;
            config.enableEncryption = false;

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
            if (argc < 4)
            {
                std::cerr << "用法: " << argv[0] << " restore <备份目录> <还原目录>\n";
                return 1;
            }

            std::string backupDir = argv[2];
            std::string restoreDir = argv[3];

            // 创建备份管理器，配置备份目录
            BackupManager::BackupConfig config;
            config.backupRoot = backupDir;

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
