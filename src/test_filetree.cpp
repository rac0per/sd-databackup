#include "core/filesystem/FileTree.h"
#include <filesystem>
#include <cassert>
#include <iostream>
#include <fstream>
namespace fs = std::filesystem;
using namespace backup::filesystem;

int main() {
    fs::path temp = fs::temp_directory_path() / "filetree_test";

    fs::remove_all(temp);
    fs::create_directories(temp / "dir1");
    fs::create_directories(temp / "dir2");

    std::ofstream(temp / "a.txt") << "hello";
    std::ofstream(temp / "dir1" / "b.txt") << "world";

    FileTree tree(temp);
    tree.build();

    tree.traverseDFS([](const FileNode& node) {
    std::cout
        << "name=" << node.getName()
        << ", relativePath=" << node.getRelativePath()
        << ", type=" << (node.isFile() ? "file" : "dir")
        << '\n';
    });


    auto node = tree.findNode("dir1/b.txt");
    assert(node);
    assert(node->isFile());

    std::cout << "FileTree test passed\n";

    fs::remove_all(temp);
}
