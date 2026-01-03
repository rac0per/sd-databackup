#pragma once

#include <filesystem>
#include <vector>
#include <memory>
#include <string>

#include "filesystem/FileTree.h"
#include "filesystem/FileTreeDiff.h"
#include "BackupMetadata.h"


namespace backup::core {

namespace fs = std::filesystem;

/*
 * BackupManager:
 *  - filesystem snapshot
 *  - diff computation
 *  - backup plan generation
 *  - plan execution
 */
class BackupManager {
public:
    // 压缩算法类型
    enum class CompressionType {
        None,
        Huffman,
        Lz77
    };

    // 加密算法类型（目前无可用算法）
    enum class EncryptionType {
        None
    };
  
    struct BackupConfig {
        fs::path sourceRoot;            // directory to back up
        fs::path backupRoot;            // destination root
        bool deleteRemoved = true;      // mirror mode
        bool dryRun = false;            // do not modify filesystem
        CompressionType compressionType = CompressionType::None;
        bool enableCompression = false; // 是否启用压缩
        // 加密配置
        EncryptionType encryptionType = EncryptionType::None;
        std::string encryptionKey; // 加密密钥
        bool enableEncryption = false;  // 是否启用加密
    };


    enum class ActionType {
        CreateDirectory,
        CopyFile,
        UpdateFile,
        RemovePath
    };
  
    struct BackupAction {
        ActionType type;
        fs::path sourcePath;   // restore 时为 backup path
        fs::path targetPath;
    };

    explicit BackupManager(BackupConfig config);

    void scan();

    std::vector<BackupAction> buildPlan();

    bool executePlan(const std::vector<BackupAction>& plan);

    void restore(const fs::path& restoreRoot);

private:
    BackupConfig config_;

    std::unique_ptr<filesystem::FileTree> sourceTree_;
    std::unique_ptr<filesystem::FileTree> backupTree_;

    std::vector<filesystem::FileChange> changes_;

    fs::path resolveSourcePath(const std::string& relativePath) const;
    fs::path resolveBackupPath(const std::string& relativePath) const;

    std::vector<BackupAction>
    translateChangesToActions(
        const std::vector<filesystem::FileChange>& changes) const;

    std::vector<BackupAction>
    translateMetadataToActions(
        const BackupMetadataInfo& metadata, const fs::path& restoreRoot) const;

    bool executeBackupAction(const BackupAction& action);
    bool executeRestoreAction(const BackupAction& action);

    bool applyCompression(
        const fs::path& input,
        const fs::path& output) const;

    bool applyDecompression(
        const fs::path& input,
        const fs::path& output) const;

    bool applyEncryption(
        const fs::path& input,
        const fs::path& output) const;

    bool applyDecryption(
        const fs::path& input,
        const fs::path& output) const;
  
    static constexpr const char* kMetadataFile = ".backupmeta";
};

} // namespace backup::core
