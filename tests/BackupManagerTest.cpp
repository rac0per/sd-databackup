#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include "backup/BackupManager.h"
using namespace backup::core;
using namespace std::filesystem;
class BackupManagerTest : public ::testing::Test {
protected:
    path sourceRoot;
    path backupRoot;
    void SetUp() override {
        sourceRoot = temp_directory_path() / "backup_source";
        backupRoot = temp_directory_path() / "backup_dest";
        create_directories(sourceRoot);
        create_directories(backupRoot);
    }
    void TearDown() override {
        remove_all(sourceRoot);
        remove_all(backupRoot);
    }
    void createTestFile(const path& filePath, const std::string& content) {
        std::ofstream(filePath) << content;
    }
    bool fileExists(const path& filePath) {
        return exists(filePath);
    }
    std::string readFileContent(const path& filePath) {
        std::ifstream file(filePath);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
};
TEST_F(BackupManagerTest, FullBackup) {
    createTestFile(sourceRoot / "file1.txt", "content1");
    createTestFile(sourceRoot / "file2.txt", "content2");
    create_directories(sourceRoot / "subdir");
    createTestFile(sourceRoot / "subdir" / "file3.txt", "content3");
    BackupManager::BackupConfig config{
        .sourceRoot = sourceRoot,
        .backupRoot = backupRoot,
        .deleteRemoved = true,
        .dryRun = false
    };
    BackupManager manager(config);
    manager.scan();
    auto plan = manager.buildPlan();
    manager.executePlan(plan);
    EXPECT_TRUE(fileExists(backupRoot / "file1.txt"));
    EXPECT_TRUE(fileExists(backupRoot / "file2.txt"));
    EXPECT_TRUE(fileExists(backupRoot / "subdir" / "file3.txt"));
    EXPECT_EQ(readFileContent(backupRoot / "file1.txt"), "content1");
    EXPECT_EQ(readFileContent(backupRoot / "file2.txt"), "content2");
    EXPECT_EQ(readFileContent(backupRoot / "subdir" / "file3.txt"), "content3");
}
TEST_F(BackupManagerTest, IncrementalBackup) {
    createTestFile(sourceRoot / "file1.txt", "content1");
    createTestFile(sourceRoot / "file2.txt", "content2");
    BackupManager::BackupConfig config{
        .sourceRoot = sourceRoot,
        .backupRoot = backupRoot,
        .deleteRemoved = true,
        .dryRun = false
    };
    BackupManager manager(config);
    manager.scan();
    auto plan = manager.buildPlan();
    manager.executePlan(plan);
    createTestFile(sourceRoot / "file1.txt", "modified content1");
    createTestFile(sourceRoot / "file3.txt", "content3");
    manager.scan();
    plan = manager.buildPlan();
    manager.executePlan(plan);
    EXPECT_EQ(readFileContent(backupRoot / "file1.txt"), "modified content1");
    EXPECT_EQ(readFileContent(backupRoot / "file2.txt"), "content2");
    EXPECT_TRUE(fileExists(backupRoot / "file3.txt"));
    EXPECT_EQ(readFileContent(backupRoot / "file3.txt"), "content3");
}
TEST_F(BackupManagerTest, MirrorBackup) {
    createTestFile(sourceRoot / "file1.txt", "content1");
    createTestFile(sourceRoot / "file2.txt", "content2");
    createTestFile(sourceRoot / "file3.txt", "content3");
    BackupManager::BackupConfig config{
        .sourceRoot = sourceRoot,
        .backupRoot = backupRoot,
        .deleteRemoved = true,
        .dryRun = false
    };
    BackupManager manager(config);
    manager.scan();
    auto plan = manager.buildPlan();
    manager.executePlan(plan);
    remove(sourceRoot / "file2.txt");
    manager.scan();
    plan = manager.buildPlan();
    manager.executePlan(plan);
    EXPECT_TRUE(fileExists(backupRoot / "file1.txt"));
    EXPECT_FALSE(fileExists(backupRoot / "file2.txt"));
    EXPECT_TRUE(fileExists(backupRoot / "file3.txt"));
}
TEST_F(BackupManagerTest, NonMirrorBackup) {
    createTestFile(sourceRoot / "file1.txt", "content1");
    createTestFile(sourceRoot / "file2.txt", "content2");
    BackupManager::BackupConfig config{
        .sourceRoot = sourceRoot,
        .backupRoot = backupRoot,
        .deleteRemoved = false,  
        .dryRun = false
    };
    BackupManager manager(config);
    manager.scan();
    auto plan = manager.buildPlan();
    manager.executePlan(plan);
    remove(sourceRoot / "file1.txt");
    manager.scan();
    plan = manager.buildPlan();
    manager.executePlan(plan);
    EXPECT_TRUE(fileExists(backupRoot / "file1.txt"));
    EXPECT_TRUE(fileExists(backupRoot / "file2.txt"));
}
TEST_F(BackupManagerTest, DryRun) {
    createTestFile(sourceRoot / "file1.txt", "content1");
    BackupManager::BackupConfig config{
        .sourceRoot = sourceRoot,
        .backupRoot = backupRoot,
        .deleteRemoved = true,
        .dryRun = true  
    };
    BackupManager manager(config);
    manager.scan();
    auto plan = manager.buildPlan();
    manager.executePlan(plan);
    EXPECT_FALSE(fileExists(backupRoot / "file1.txt"));
    EXPECT_GT(plan.size(), 0);
}
TEST_F(BackupManagerTest, ActionTranslation) {
    createTestFile(sourceRoot / "file1.txt", "content1");
    createTestFile(sourceRoot / "file2.txt", "content2");
    BackupManager::BackupConfig config{
        .sourceRoot = sourceRoot,
        .backupRoot = backupRoot,
        .deleteRemoved = true,
        .dryRun = false
    };
    BackupManager manager(config);
    manager.scan();
    auto plan = manager.buildPlan();
    bool hasCreateDir = false;
    bool hasCopyFile = false;
    for (const auto& action : plan) {
        if (action.type == BackupManager::ActionType::CreateDirectory) {
            hasCreateDir = true;
        } else if (action.type == BackupManager::ActionType::CopyFile) {
            hasCopyFile = true;
        }
    }
    EXPECT_TRUE(hasCreateDir || hasCopyFile); 
}