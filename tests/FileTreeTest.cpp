#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "filesystem/FileTree.h"
using namespace backup::filesystem;
using namespace std::filesystem;
class FileTreeTest : public ::testing::Test {
protected:
    path testRoot;
    void SetUp() override {
        testRoot = temp_directory_path() / "backup_test";
        create_directories(testRoot);
        std::ofstream(testRoot / "file1.txt") << "content1";
        std::ofstream(testRoot / "file2.txt") << "content2";
        create_directories(testRoot / "subdir");
        std::ofstream(testRoot / "subdir" / "file3.txt") << "content3";
    }
    void TearDown() override {
        remove_all(testRoot);
    }
};
TEST_F(FileTreeTest, BasicProperties) {
    FileTree fileTree(testRoot);
    EXPECT_EQ(fileTree.getRootPath(), testRoot);
    EXPECT_EQ(fileTree.getRoot(), nullptr);
}
TEST_F(FileTreeTest, BuildTree) {
    FileTree fileTree(testRoot);
    fileTree.build();
    auto root = fileTree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(root->isDirectory());
    EXPECT_EQ(root->getName(), "backup_test");
    EXPECT_EQ(root->getRelativePath(), ".");
    auto rootChildren = root->getChildren();
    EXPECT_EQ(rootChildren.size(), 3); 
}
TEST_F(FileTreeTest, DFSTraversal) {
    FileTree fileTree(testRoot);
    fileTree.build();
    std::vector<std::string> visitedPaths;
    fileTree.traverseDFS([&visitedPaths](const FileNode& node) {
        visitedPaths.push_back(node.getRelativePath());
    });
    EXPECT_NE(std::find(visitedPaths.begin(), visitedPaths.end(), "."), visitedPaths.end());
    EXPECT_NE(std::find(visitedPaths.begin(), visitedPaths.end(), "file1.txt"), visitedPaths.end());
    EXPECT_NE(std::find(visitedPaths.begin(), visitedPaths.end(), "file2.txt"), visitedPaths.end());
    EXPECT_NE(std::find(visitedPaths.begin(), visitedPaths.end(), "subdir"), visitedPaths.end());
    EXPECT_NE(std::find(visitedPaths.begin(), visitedPaths.end(), "subdir/file3.txt"), visitedPaths.end());
    EXPECT_EQ(visitedPaths.size(), 5);
}
TEST(FileTreeEmptyDirTest, EmptyDirectory) {
    auto emptyDir = temp_directory_path() / "empty_backup_test";
    create_directories(emptyDir);
    FileTree fileTree(emptyDir);
    fileTree.build();
    auto root = fileTree.getRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_TRUE(root->isDirectory());
    EXPECT_EQ(root->getChildren().size(), 0);
    remove_all(emptyDir);
}