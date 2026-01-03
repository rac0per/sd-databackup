#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "filesystem/FileTreeDiff.h"
using namespace backup::filesystem;
using namespace std::filesystem;
class FileTreeDiffTest : public ::testing::Test {
protected:
    path oldRoot;
    path newRoot;
    void SetUp() override {
        oldRoot = temp_directory_path() / "backup_old";
        create_directories(oldRoot);
        std::ofstream(oldRoot / "file1.txt") << "content1";
        create_directories(oldRoot / "subdir");
        std::ofstream(oldRoot / "subdir" / "file2.txt") << "content2";
        newRoot = temp_directory_path() / "backup_new";
        create_directories(newRoot);
    }
    void TearDown() override {
        remove_all(oldRoot);
        remove_all(newRoot);
    }
    void buildTree(FileTree& tree) {
        tree.build();
    }
};
TEST_F(FileTreeDiffTest, IdenticalTrees) {
    copy(oldRoot, newRoot, copy_options::recursive);
    for (const auto& entry : recursive_directory_iterator(oldRoot)) {
        if (is_regular_file(entry)) {
            path oldPath = entry.path();
            path newPath = newRoot / oldPath.lexically_relative(oldRoot);
            if (exists(newPath)) {
                last_write_time(newPath, last_write_time(oldPath));
            }
        }
    }
    FileTree oldTree(oldRoot);
    buildTree(oldTree);
    FileTree newTree(newRoot);
    buildTree(newTree);
    auto changes = FileTreeDiff::diff(oldTree, newTree);
    EXPECT_TRUE(changes.empty());
}
TEST_F(FileTreeDiffTest, AddedFiles) {
    copy(oldRoot, newRoot, copy_options::recursive);
    std::ofstream(newRoot / "file3.txt") << "content3";
    std::ofstream(newRoot / "subdir" / "file4.txt") << "content4";
    create_directories(newRoot / "newdir");
    FileTree oldTree(oldRoot);
    buildTree(oldTree);
    FileTree newTree(newRoot);
    buildTree(newTree);
    auto changes = FileTreeDiff::diff(oldTree, newTree);
    EXPECT_EQ(changes.size(), 3);
    for (const auto& change : changes) {
        EXPECT_EQ(change.type, ChangeType::Added);
    }
}
TEST_F(FileTreeDiffTest, RemovedFiles) {
    copy(oldRoot, newRoot, copy_options::recursive);
    remove(newRoot / "file1.txt");
    remove(newRoot / "subdir" / "file2.txt");
    FileTree oldTree(oldRoot);
    buildTree(oldTree);
    FileTree newTree(newRoot);
    buildTree(newTree);
    auto changes = FileTreeDiff::diff(oldTree, newTree);
    EXPECT_EQ(changes.size(), 2);
    for (const auto& change : changes) {
        EXPECT_EQ(change.type, ChangeType::Removed);
    }
}
TEST_F(FileTreeDiffTest, ModifiedFiles) {
    copy(oldRoot, newRoot, copy_options::recursive);
    std::ofstream(newRoot / "file1.txt") << "modified content1";
    FileTree oldTree(oldRoot);
    buildTree(oldTree);
    FileTree newTree(newRoot);
    buildTree(newTree);
    auto changes = FileTreeDiff::diff(oldTree, newTree);
    EXPECT_EQ(changes.size(), 1);
    EXPECT_EQ(changes[0].type, ChangeType::Modified);
    EXPECT_EQ(changes[0].relativePath, "file1.txt");
}
TEST_F(FileTreeDiffTest, MixedChanges) {
    copy(oldRoot, newRoot, copy_options::recursive);
    remove(newRoot / "file1.txt");
    std::ofstream(newRoot / "subdir" / "file2.txt") << "modified content2";
    std::ofstream(newRoot / "file3.txt") << "content3";
    create_directories(newRoot / "newdir");
    std::ofstream(newRoot / "newdir" / "file4.txt") << "content4";
    FileTree oldTree(oldRoot);
    buildTree(oldTree);
    FileTree newTree(newRoot);
    buildTree(newTree);
    auto changes = FileTreeDiff::diff(oldTree, newTree);
    EXPECT_EQ(changes.size(), 5);
    int addedCount = 0;
    int removedCount = 0;
    int modifiedCount = 0;
    for (const auto& change : changes) {
        switch (change.type) {
            case ChangeType::Added:
                addedCount++;
                break;
            case ChangeType::Removed:
                removedCount++;
                break;
            case ChangeType::Modified:
                modifiedCount++;
                break;
        }
    }
    EXPECT_EQ(addedCount, 3);   
    EXPECT_EQ(removedCount, 1); 
    EXPECT_EQ(modifiedCount, 1); 
}