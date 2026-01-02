#include "BackupManager.h"

#include <stdexcept>
#include <filesystem>
#include <iostream>

namespace backup::core {

using filesystem::FileTree;
using filesystem::FileTreeDiff;
using filesystem::FileChange;
using filesystem::ChangeType;
namespace fs = std::filesystem;

BackupManager::BackupManager(BackupConfig config)
    : config_(std::move(config)) {}

void BackupManager::scan() {
    if (!fs::exists(config_.sourceRoot) || !fs::is_directory(config_.sourceRoot)) {
        throw std::runtime_error("Invalid source root");
    }

    if (!fs::exists(config_.backupRoot) || !fs::is_directory(config_.backupRoot)) {
        throw std::runtime_error("Invalid backup root");
    }

    sourceTree_ = std::make_unique<FileTree>(config_.sourceRoot);
    backupTree_ = std::make_unique<FileTree>(config_.backupRoot);

    sourceTree_->build();
    backupTree_->build();
}

std::vector<BackupManager::BackupAction> BackupManager::buildPlan() {
    if (!sourceTree_ || !backupTree_) {
        throw std::runtime_error("scan() must be called before buildPlan()");
    }

    changes_ = FileTreeDiff::diff(*backupTree_, *sourceTree_);

    return translateChangesToActions(changes_);
}

void BackupManager::executePlan(const std::vector<BackupAction>& plan) {
    for (const auto& action : plan) {
        executeAction(action);
    }
}

fs::path BackupManager::resolveSourcePath(const std::string& relativePath) const {
    return config_.sourceRoot / relativePath;
}

fs::path BackupManager::resolveBackupPath(const std::string& relativePath) const {
    return config_.backupRoot / relativePath;
}
std::vector<BackupManager::BackupAction>
BackupManager::translateChangesToActions(
    const std::vector<filesystem::FileChange>& changes) const {

    std::vector<BackupAction> actions;

    for (const auto& change : changes) {
        const auto& rel = change.relativePath;

        switch (change.type) {
        case filesystem::ChangeType::Added:
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

        case filesystem::ChangeType::Modified:
            // Modified 在 diff 中已保证是文件
            actions.push_back({
                ActionType::UpdateFile,
                resolveSourcePath(rel),
                resolveBackupPath(rel)
            });
            break;

        case filesystem::ChangeType::Removed:
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


void BackupManager::executeAction(const BackupAction& action) {
    if (config_.dryRun) {
        return;
    }

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
        break;

    case ActionType::RemovePath:
        if (fs::exists(action.targetPath)) {
            fs::remove_all(action.targetPath);
        }
        break;

    default:
        throw std::runtime_error("Unknown BackupAction type");
    }
}

} // namespace backup::core
