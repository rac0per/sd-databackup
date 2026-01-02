#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include "FileTree.h"

namespace backup::filesystem {

enum class ChangeType {
    Added,
    Removed,
    Modified
};

struct FileChange {
    ChangeType type;
    std::string relativePath;
    std::shared_ptr<FileNode> oldNode;
    std::shared_ptr<FileNode> newNode;
};

class FileTreeDiff {
public:
    static std::vector<FileChange>
    diff(const FileTree& oldTree, const FileTree& newTree);

private:
    using NodeMap =
        std::unordered_map<std::string, std::shared_ptr<FileNode>>;

    static NodeMap flatten(const FileTree& tree);
    static bool isSameFile(const FileNode& a, const FileNode& b);
};

} // namespace backup::filesystem
