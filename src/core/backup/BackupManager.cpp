#include "BackupManager.h"

#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace backup::core
{

    using filesystem::ChangeType;
    using filesystem::FileChange;
    using filesystem::FileTree;
    using filesystem::FileTreeDiff;
    namespace fs = std::filesystem;

    BackupManager::BackupManager(BackupConfig config)
        : config_(std::move(config)) {}

    void BackupManager::scan()
    {
        if (!fs::exists(config_.sourceRoot) || !fs::is_directory(config_.sourceRoot))
        {
            throw std::runtime_error("源目录无效：" + config_.sourceRoot.string());
        }

        // 如果备份目录不存在，创建它
        if (!fs::exists(config_.backupRoot))
        {
            fs::create_directories(config_.backupRoot);
        }
        else if (!fs::is_directory(config_.backupRoot))
        {
            throw std::runtime_error("备份目录无效：" + config_.backupRoot.string());
        }

        sourceTree_ = std::make_unique<FileTree>(config_.sourceRoot);
        backupTree_ = std::make_unique<FileTree>(config_.backupRoot);

        sourceTree_->build();
        backupTree_->build();
    }

    std::vector<BackupManager::BackupAction> BackupManager::buildPlan()
    {
        if (!sourceTree_ || !backupTree_)
        {
            throw std::runtime_error("必须先执行scan()方法，再执行buildPlan()方法");
        }

        changes_ = FileTreeDiff::diff(*backupTree_, *sourceTree_);

        return translateChangesToActions(changes_);
    }

    void BackupManager::executePlan(const std::vector<BackupAction> &plan)
    {
        for (const auto &action : plan)
        {
            executeAction(action);
        }
    }

    fs::path BackupManager::resolveSourcePath(const std::string &relativePath) const
    {
        return config_.sourceRoot / relativePath;
    }

    fs::path BackupManager::resolveBackupPath(const std::string &relativePath) const
    {
        return config_.backupRoot / relativePath;
    }
    std::vector<BackupManager::BackupAction>
    BackupManager::translateChangesToActions(
        const std::vector<filesystem::FileChange> &changes) const
    {

        std::vector<BackupAction> actions;

        for (const auto &change : changes)
        {
            const auto &rel = change.relativePath;

            switch (change.type)
            {
            case filesystem::ChangeType::Added:
                if (change.newNode && change.newNode->isDirectory())
                {
                    actions.push_back({ActionType::CreateDirectory,
                                       {},
                                       resolveBackupPath(rel)});
                }
                else
                {
                    actions.push_back({ActionType::CopyFile,
                                       resolveSourcePath(rel),
                                       resolveBackupPath(rel)});
                }
                break;

            case filesystem::ChangeType::Modified:
                // Modified 在 diff 中已保证是文件
                actions.push_back({ActionType::UpdateFile,
                                   resolveSourcePath(rel),
                                   resolveBackupPath(rel)});
                break;

            case filesystem::ChangeType::Removed:
                if (config_.deleteRemoved && change.oldNode)
                {
                    actions.push_back({ActionType::RemovePath,
                                       {},
                                       resolveBackupPath(rel)});
                }
                break;

            default:
                break;
            }
        }

        return actions;
    }

    void BackupManager::executeAction(const BackupAction &action)
    {
        if (config_.dryRun)
        {
            return;
        }

        switch (action.type)
        {
        case ActionType::CreateDirectory:
            fs::create_directories(action.targetPath);
            break;

        case ActionType::CopyFile:
        case ActionType::UpdateFile:
        {
            fs::create_directories(action.targetPath.parent_path());

            // 目前暂时只支持直接复制文件，不支持压缩和加密
            fs::copy_file(
                action.sourcePath,
                action.targetPath,
                fs::copy_options::overwrite_existing);
            break;
        }

        case ActionType::RemovePath:
            if (fs::exists(action.targetPath))
            {
                fs::remove_all(action.targetPath);
            }
            break;

        default:
            throw std::runtime_error("未知的备份操作类型");
        }
    }

} // namespace backup::core
