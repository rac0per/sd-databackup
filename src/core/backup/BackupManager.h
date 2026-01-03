#pragma once

#include <filesystem>
#include <vector>
#include <memory>

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
    struct BackupConfig {
        fs::path sourceRoot;            // directory to back up
        fs::path backupRoot;            // destination root
        bool deleteRemoved = true;      // mirror mode
        bool dryRun = false;            // do not modify filesystem
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

    static constexpr const char* kMetadataFile = ".backupmeta";
};

} // namespace backup::core
