#pragma once

#include <filesystem>
#include <vector>
#include <memory>
#include <string>

#include "filesystem/FileTree.h"
#include "filesystem/FileTreeDiff.h"

namespace backup::core
{

    namespace fs = std::filesystem;

    // 压缩算法类型
    enum class CompressionType
    {
        None,
        Gzip,
        Bzip2,
        Lz4
    };

    // 加密算法类型
    enum class EncryptionType
    {
        None,
        AES,
        DES,
        RSA
    };

    /*
     * BackupManager:
     *  - filesystem snapshot
     *  - diff computation
     *  - backup plan generation
     *  - plan execution
     */
    class BackupManager
    {
    public:
        struct BackupConfig
        {
            fs::path sourceRoot;       // directory to back up
            fs::path backupRoot;       // destination root
            bool deleteRemoved = true; // mirror mode
            bool dryRun = false;       // do not modify filesystem

            // 压缩配置
            CompressionType compressionType = CompressionType::None;

            // 加密配置
            EncryptionType encryptionType = EncryptionType::None;
            std::string encryptionKey; // 加密密钥
        };

        enum class ActionType
        {
            CreateDirectory,
            CopyFile,
            UpdateFile,
            RemovePath
        };

        struct BackupAction
        {
            ActionType type;
            fs::path sourcePath; // may be empty for Remove
            fs::path targetPath;
        };

        explicit BackupManager(BackupConfig config);

        // Build snapshot of source & destination
        void scan();

        // Compute diff and generate executable plan
        std::vector<BackupAction> buildPlan();

        // Execute plan (respecting dryRun)
        void executePlan(const std::vector<BackupAction> &plan);

    private:
        BackupConfig config_;

        std::unique_ptr<filesystem::FileTree> sourceTree_;
        std::unique_ptr<filesystem::FileTree> backupTree_;

        std::vector<filesystem::FileChange> changes_;

        fs::path resolveSourcePath(const std::string &relativePath) const;
        fs::path resolveBackupPath(const std::string &relativePath) const;

        std::vector<BackupAction>
        translateChangesToActions(
            const std::vector<filesystem::FileChange> &changes) const;

        void executeAction(const BackupAction &action);
    };

} // namespace backup::core
