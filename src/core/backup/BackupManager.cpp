#include "BackupManager.h"
#include "BackupMetadata.h"
#include "compression/Compression.h"
#include "encryption/Encryption.h"

#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace backup::core {

using filesystem::ChangeType;
using filesystem::FileChange;
using filesystem::FileTree;
using filesystem::FileTreeDiff;
namespace fs = std::filesystem;

BackupManager::BackupManager(BackupConfig config)
    : config_(std::move(config)) {}

namespace {
// Map BackupManager::CompressionType to compression::CompressionType safely.
inline backup::core::compression::CompressionType toCompressionAlgo(
    BackupManager::CompressionType t) {
    switch (t) {
    case BackupManager::CompressionType::Huffman:
        return backup::core::compression::CompressionType::Huffman;
    case BackupManager::CompressionType::Lz77:
        return backup::core::compression::CompressionType::Lz77;
    default:
        throw std::invalid_argument("Invalid compression type");
    }
}
}

void BackupManager::scan() {
    if (!fs::exists(config_.sourceRoot) || !fs::is_directory(config_.sourceRoot)) {
        throw std::runtime_error("Invalid source root");
    }

    if (!fs::exists(config_.backupRoot)) {
        fs::create_directories(config_.backupRoot);
    } else if (!fs::is_directory(config_.backupRoot)) {
        throw std::runtime_error("Invalid backup root");
    }

    sourceTree_ = std::make_unique<FileTree>(config_.sourceRoot);
    backupTree_ = std::make_unique<FileTree>(config_.backupRoot);

    sourceTree_->build();
    backupTree_->build();
}

std::vector<BackupManager::BackupAction> BackupManager::buildPlan() {
    if (!sourceTree_ || !backupTree_) {
        throw std::runtime_error("必须先执行 scan()，再执行 buildPlan()");
    }

    changes_ = FileTreeDiff::diff(*backupTree_, *sourceTree_);
    return translateChangesToActions(changes_);
}

bool BackupManager::executePlan(const std::vector<BackupAction>& plan) {
    bool success = true;

    for (const auto& action : plan) {
        if (!executeBackupAction(action)) {
            success = false;
        }
    }

    if (success && !config_.dryRun) {
        // 仅当启用了相应功能时记录算法名称，否则记录为 none
        std::string compressionStr = "none";
        std::string encryptionStr = "none";

        if (config_.enableCompression) {
            switch (config_.compressionType) {
            case CompressionType::Huffman: compressionStr = "huffman"; break;
            case CompressionType::Lz77: compressionStr = "lz77"; break;
            default: compressionStr = "none"; break;
            }
        }

        // 暂无可用加密算法，保持 none

        BackupMetadata::writeMetadata(*sourceTree_, config_.backupRoot, compressionStr, encryptionStr);
    }

    return success;
}

fs::path BackupManager::resolveSourcePath(const std::string& relativePath) const {
    return config_.sourceRoot / relativePath;
}

fs::path BackupManager::resolveBackupPath(const std::string& relativePath) const {
    return config_.backupRoot / relativePath;
}

std::vector<BackupManager::BackupAction>
BackupManager::translateChangesToActions(const std::vector<filesystem::FileChange>& changes) const {

    std::vector<BackupAction> actions;
    for (const auto& change : changes) {
        const auto& rel = change.relativePath;

        switch (change.type) {
        case ChangeType::Added:
            if (change.newNode && change.newNode->isDirectory()) {
                actions.push_back({
                    ActionType::CreateDirectory,
                    {},
                    resolveBackupPath(rel)
                });
            } else {
                actions.push_back({
                    ActionType::CopyFile,
                    resolveSourcePath(rel),
                    resolveBackupPath(rel)
                });
            }
            break;

        case ChangeType::Modified:
            actions.push_back({
                ActionType::UpdateFile,
                resolveSourcePath(rel),
                resolveBackupPath(rel)
            });
            break;

        case ChangeType::Removed:
            if (config_.deleteRemoved && change.oldNode) {
                actions.push_back({
                    ActionType::RemovePath,
                    {},
                    resolveBackupPath(rel)
                });
            }
            break;

        default:
            break;
        }
    }

    return actions;
}

bool BackupManager::executeBackupAction(const BackupAction& action) {
    if (config_.dryRun) return true;

    try {
        switch (action.type) {
        case ActionType::CreateDirectory:
            fs::create_directories(action.targetPath);
            break;

        case ActionType::CopyFile:
        case ActionType::UpdateFile: {
            fs::create_directories(action.targetPath.parent_path());

            fs::path current = action.sourcePath;
            fs::path tempCompressed = action.targetPath; tempCompressed += ".tmp_compress";
            fs::path tempEncrypted  = action.targetPath; tempEncrypted  += ".tmp_encrypt";

            // 压缩（可选）
            if (config_.enableCompression && config_.compressionType != CompressionType::None) {
                if (!applyCompression(current, tempCompressed)) {
                    fs::remove(tempCompressed);
                    return false;
                }
                current = tempCompressed;
            }

            // 加密（可选）
            if (config_.enableEncryption && config_.encryptionType != EncryptionType::None) {
                std::cerr << "[Encryption] no encryption algorithms available\n";
                fs::remove(tempCompressed);
                fs::remove(tempEncrypted);
                return false;
            }

            // 写入最终目标
            if (current == action.sourcePath) {
                fs::copy_file(
                    action.sourcePath,
                    action.targetPath,
                    fs::copy_options::overwrite_existing
                );
            } else {
                fs::rename(current, action.targetPath);
            }

            fs::remove(tempCompressed);
            fs::remove(tempEncrypted);

            // 设置文件权限和时间戳
            fs::permissions(
                action.targetPath,
                fs::status(action.sourcePath).permissions()
            );
            fs::last_write_time(
                action.targetPath,
                fs::last_write_time(action.sourcePath)
            );
            break;
        }

        case ActionType::RemovePath:
            fs::remove_all(action.targetPath);
            break;

        default:
            throw std::runtime_error("Unknown backup action");
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Backup] failed: " << e.what() << "\n";
        return false;
    }
}

void BackupManager::restore(const fs::path& restoreRoot) {
    auto metadata = BackupMetadata::readMetadata(config_.backupRoot);
    
    // 从metadata中读取压缩和加密类型
    if (metadata.compressionType == "huffman") {
        config_.compressionType = CompressionType::Huffman;
        config_.enableCompression = true;
    } else if (metadata.compressionType == "lz77") {
        config_.compressionType = CompressionType::Lz77;
        config_.enableCompression = true;
    } else {
        config_.compressionType = CompressionType::None;
        config_.enableCompression = false;
    }
    
    // 当前无加密算法可用
    config_.encryptionType = EncryptionType::None;
    config_.enableEncryption = false;
    
    auto actions = translateMetadataToActions(metadata, fs::absolute(restoreRoot));

    for (const auto& action : actions) {
        executeRestoreAction(action);
    }
}

std::vector<BackupManager::BackupAction>
BackupManager::translateMetadataToActions(
    const BackupMetadataInfo& metadata,
    const fs::path& restoreRoot) const {

    std::vector<BackupAction> dirs;
    std::vector<BackupAction> files;

    for (const auto& entry : metadata.files) {
        if (entry.relativePath == kMetadataFile) continue;

        fs::path src = config_.backupRoot / entry.relativePath;
        fs::path dst = restoreRoot / entry.relativePath;

        if (entry.isDirectory) {
            dirs.push_back({ ActionType::CreateDirectory, {}, dst });
        } else {
            files.push_back({ ActionType::CopyFile, src, dst });
        }
    }

    dirs.insert(dirs.end(), files.begin(), files.end());
    return dirs;
}

bool BackupManager::executeRestoreAction(const BackupAction& action) {
    if (config_.dryRun) return true;

    try {
        switch (action.type) {
        case ActionType::CreateDirectory:
            fs::create_directories(action.targetPath);
            break;

        case ActionType::CopyFile: {
            fs::create_directories(action.targetPath.parent_path());

            fs::path current = action.sourcePath;
            fs::path tempDecrypted = action.targetPath; tempDecrypted += ".tmp_decrypt";
            fs::path tempDecompressed = action.targetPath; tempDecompressed += ".tmp_decompress";

            // 解密（可选）
            if (config_.enableEncryption && config_.encryptionType != EncryptionType::None) {
                std::cerr << "[Encryption] no encryption algorithms available\n";
                fs::remove(tempDecrypted);
                return false;
            }
            // 未开启加密直接复制到解密阶段的临时文件，便于后续统一处理
            fs::copy_file(action.sourcePath, tempDecrypted, fs::copy_options::overwrite_existing);
            current = tempDecrypted;

            // 解压（可选）
            if (config_.enableCompression && config_.compressionType != CompressionType::None) {
                if (!applyDecompression(current, tempDecompressed)) {
                    fs::remove(tempDecrypted);
                    fs::remove(tempDecompressed);
                    return false;
                }
                if (current == tempDecrypted) fs::remove(tempDecrypted);
                current = tempDecompressed;
            }

            // 写入最终目标
            if (current == action.sourcePath) {
                fs::copy_file(action.sourcePath, action.targetPath, fs::copy_options::overwrite_existing);
            } else {
                fs::rename(current, action.targetPath);
            }

            fs::remove(tempDecrypted);
            fs::remove(tempDecompressed);

            // 恢复备份文件的权限与时间戳
            fs::permissions(
                action.targetPath,
                fs::status(action.sourcePath).permissions()
            );
            fs::last_write_time(
                action.targetPath,
                fs::last_write_time(action.sourcePath)
            );
            break;

        }

        default:
            break;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Restore] failed: " << e.what() << "\n";
        return false;
    }
}

bool BackupManager::applyCompression(const fs::path& input, const fs::path& output) const {
    if (config_.compressionType == CompressionType::None) {
        // 直接复制文件
        fs::copy_file(input, output, fs::copy_options::overwrite_existing);
        return true;
    }

    try {
        // 创建对应的压缩器
        auto compressor = backup::core::compression::createCompressor(
            toCompressionAlgo(config_.compressionType)
        );
        compressor->compress(input, output);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Compression] failed: " << e.what() << "\n";
        return false;
    }
}

bool BackupManager::applyDecompression(const fs::path& input, const fs::path& output) const {
    if (config_.compressionType == CompressionType::None) {
        // 直接复制文件
        fs::copy_file(input, output, fs::copy_options::overwrite_existing);
        return true;
    }

    try {
        // 创建对应的解压器
        auto compressor = backup::core::compression::createCompressor(
            toCompressionAlgo(config_.compressionType)
        );
        compressor->decompress(input, output);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Decompression] failed: " << e.what() << "\n";
        return false;
    }
}

bool BackupManager::applyEncryption(const fs::path& input, const fs::path& output) const {
    // 当前无加密算法，直接返回失败以触发上层保护逻辑
    std::cerr << "[Encryption] no encryption algorithms available\n";
    return false;
}

bool BackupManager::applyDecryption(const fs::path& input, const fs::path& output) const {
    std::cerr << "[Decryption] no encryption algorithms available\n";
    return false;
}

} // namespace backup::core
