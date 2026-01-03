#include <gtest/gtest.h>
#include "filesystem/FileNode.h"
using namespace backup::filesystem;
class FileNodeTest : public ::testing::Test {
protected:
    FileNode testFile{"test.txt", "test.txt", FileType::File, 100, FileNode::FileTime{}};
    FileNode testDir{"test_dir", "test_dir", FileType::Directory};
};
TEST_F(FileNodeTest, NodeTypeCheck) {
    EXPECT_TRUE(testFile.isFile());
    EXPECT_FALSE(testFile.isDirectory());
    EXPECT_FALSE(testDir.isFile());
    EXPECT_TRUE(testDir.isDirectory());
}
TEST_F(FileNodeTest, GetAttributes) {
    EXPECT_EQ(testFile.getName(), "test.txt");
    EXPECT_EQ(testFile.getRelativePath(), "test.txt");
    EXPECT_EQ(testFile.getSize(), 100);
    EXPECT_EQ(testDir.getName(), "test_dir");
    EXPECT_EQ(testDir.getRelativePath(), "test_dir");
}
TEST_F(FileNodeTest, ChildrenManagement) {
    EXPECT_TRUE(testDir.getChildren().empty());
    auto childFile = std::make_shared<FileNode>("child.txt", "test_dir/child.txt", FileType::File);
    testDir.addChild(childFile);
    EXPECT_EQ(testDir.getChildren().size(), 1);
    EXPECT_EQ(testDir.getChildren()[0]->getName(), "child.txt");
    EXPECT_EQ(testDir.getChildren()[0]->getRelativePath(), "test_dir/child.txt");
    auto childDir = std::make_shared<FileNode>("grandchild_dir", "test_dir/grandchild_dir", FileType::Directory);
    testDir.addChild(childDir);
    EXPECT_EQ(testDir.getChildren().size(), 2);
}
TEST(FileNodeConstructorTest, FileNodeCreation) {
    FileNode defaultFile{"default.txt", "default.txt", FileType::File};
    EXPECT_EQ(defaultFile.getSize(), 0);
    auto currentTime = FileNode::FileTime{};
    FileNode fullFile{"full.txt", "full.txt", FileType::File, 500, currentTime};
    EXPECT_EQ(fullFile.getName(), "full.txt");
    EXPECT_EQ(fullFile.getSize(), 500);
    EXPECT_EQ(fullFile.getMTime(), currentTime);
}