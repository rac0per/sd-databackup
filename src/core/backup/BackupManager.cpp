#include "BackupManager.h"
#include "BackupMetadata.h"

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
        BackupMetadata::writeMetadata(*sourceTree_, config_.backupRoot);
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
        case ActionType::UpdateFile:
            fs::create_directories(action.targetPath.parent_path());
            fs::copy_file(
                action.sourcePath,
                action.targetPath,
                fs::copy_options::overwrite_existing
            );

            fs::permissions(
                action.targetPath,
                fs::status(action.sourcePath).permissions()
            );
            fs::last_write_time(
                action.targetPath,
                fs::last_write_time(action.sourcePath)
            );
            break;

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

        case ActionType::CopyFile:
            fs::create_directories(action.targetPath.parent_path());
            fs::copy_file(
                action.sourcePath,
                action.targetPath,
                fs::copy_options::overwrite_existing
            );
            break;

        default:
            break;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Restore] failed: " << e.what() << "\n";
        return false;
    }
}

} // namespace backup::core
