#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "backup/BackupManager.h"

using namespace backup::core;
namespace fs = std::filesystem;

namespace {
void writeFile(const fs::path& p, const std::string& content) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << content;
}

std::string readFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::string readMetaValue(const fs::path& meta, const std::string& key) {
    std::ifstream in(meta);
    std::string line;
    const std::string prefix = key + "=";
    while (std::getline(in, line)) {
        if (line.rfind(prefix, 0) == 0) return line.substr(prefix.size());
    }
    return {};
}
}

class BackupManagerTest : public ::testing::Test {
protected:
    fs::path sourceRoot;
    fs::path backupRoot;
    fs::path restoreRoot;

    void SetUp() override {
        sourceRoot = fs::temp_directory_path() / "bm_src";
        backupRoot = fs::temp_directory_path() / "bm_dst";
        restoreRoot = fs::temp_directory_path() / "bm_restore";
        fs::create_directories(sourceRoot);
        fs::create_directories(backupRoot);
    }

    void TearDown() override {
        fs::remove_all(sourceRoot);
        fs::remove_all(backupRoot);
        fs::remove_all(restoreRoot);
    }
};

TEST_F(BackupManagerTest, BackupAndRestoreWithoutCompression) {
    writeFile(sourceRoot / "file1.txt", "a");
    writeFile(sourceRoot / "sub/inner.txt", "b");

    BackupManager::BackupConfig config{};
    config.sourceRoot = sourceRoot;
    config.backupRoot = backupRoot;
    config.deleteRemoved = true;
    config.enableCompression = false;

    BackupManager mgr(config);
    mgr.scan();
    auto plan = mgr.buildPlan();
    mgr.executePlan(plan);

    EXPECT_TRUE(fs::exists(backupRoot / "file1.txt"));
    EXPECT_TRUE(fs::exists(backupRoot / "sub/inner.txt"));
    EXPECT_TRUE(fs::exists(backupRoot / ".backupmeta"));
    EXPECT_EQ(readFile(backupRoot / "file1.txt"), "a");

    BackupManager::BackupConfig restoreCfg{};
    restoreCfg.backupRoot = backupRoot;
    BackupManager restoreMgr(restoreCfg);
    restoreMgr.restore(restoreRoot);

    EXPECT_EQ(readFile(restoreRoot / "file1.txt"), "a");
    EXPECT_EQ(readFile(restoreRoot / "sub/inner.txt"), "b");

    auto srcMtime = fs::last_write_time(sourceRoot / "file1.txt");
    auto restoredMtime = fs::last_write_time(restoreRoot / "file1.txt");
    EXPECT_EQ(srcMtime, restoredMtime);
}

TEST_F(BackupManagerTest, BackupWithCompressionThenRestoreHuffman) {
    writeFile(sourceRoot / "data.bin", std::string(1024, 'x'));

    BackupManager::BackupConfig config{};
    config.sourceRoot = sourceRoot;
    config.backupRoot = backupRoot;
    config.deleteRemoved = true;
    config.enableCompression = true;
    config.compressionType = BackupManager::CompressionType::Huffman;

    BackupManager mgr(config);
    mgr.scan();
    auto plan = mgr.buildPlan();
    mgr.executePlan(plan);

    EXPECT_EQ(readMetaValue(backupRoot / ".backupmeta", "compression"), "huffman");

    BackupManager::BackupConfig restoreCfg{};
    restoreCfg.backupRoot = backupRoot;
    BackupManager restoreMgr(restoreCfg);
    restoreMgr.restore(restoreRoot);

    EXPECT_EQ(readFile(restoreRoot / "data.bin"), std::string(1024, 'x'));
}

TEST_F(BackupManagerTest, MirrorDeletesRemovedFiles) {
    writeFile(sourceRoot / "keep.txt", "keep");
    writeFile(sourceRoot / "drop.txt", "drop");

    BackupManager::BackupConfig config{};
    config.sourceRoot = sourceRoot;
    config.backupRoot = backupRoot;
    config.deleteRemoved = true;

    BackupManager mgr(config);
    mgr.scan();
    auto plan = mgr.buildPlan();
    mgr.executePlan(plan);

    fs::remove(sourceRoot / "drop.txt");
    writeFile(sourceRoot / "new.txt", "new");

    mgr.scan();
    plan = mgr.buildPlan();
    mgr.executePlan(plan);

    EXPECT_TRUE(fs::exists(backupRoot / "keep.txt"));
    EXPECT_FALSE(fs::exists(backupRoot / "drop.txt"));
    EXPECT_TRUE(fs::exists(backupRoot / "new.txt"));
}

TEST_F(BackupManagerTest, DryRunDoesNotModifyFilesystem) {
    writeFile(sourceRoot / "file.txt", "dry");

    BackupManager::BackupConfig config{};
    config.sourceRoot = sourceRoot;
    config.backupRoot = backupRoot;
    config.dryRun = true;

    BackupManager mgr(config);
    mgr.scan();
    auto plan = mgr.buildPlan();
    auto success = mgr.executePlan(plan);

    EXPECT_TRUE(success);
    EXPECT_FALSE(fs::exists(backupRoot / "file.txt"));
    EXPECT_FALSE(fs::exists(backupRoot / ".backupmeta"));
    EXPECT_FALSE(plan.empty());
}

TEST_F(BackupManagerTest, RestoreFailsWithoutMetadata) {
    BackupManager::BackupConfig restoreCfg{};
    restoreCfg.backupRoot = backupRoot; // empty, no .backupmeta
    BackupManager restoreMgr(restoreCfg);
    EXPECT_THROW(restoreMgr.restore(restoreRoot), std::runtime_error);
}