#pragma once
#include "FileTree.h"
#include "FileNode.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace backup::filesystem {
enum class ChangeType {
    Added,      // 仅存在于 newTree
    Removed,    // 仅存在于 oldTree
    Modified,   // 两边都有，但内容/属性不同
    Unchanged   // 两边都有，且相同
};

struct FileChange {
    ChangeType type;
    std::string relativePath;

    std::shared_ptr<FileNode> oldNode; // Removed / Modified 时有效
    std::shared_ptr<FileNode> newNode; // Added / Modified 时有效
};

using ChangeSet = std::vector<FileChange>;
// using NodeMap = std::unordered_map<std::string, std::shared_ptr<FileNode>>;

class FileTreeDiff {
public:
    static ChangeSet diff(const FileTree& oldTree, const FileTree& newTree);
private:
    using NodeMap = std::unordered_map<std::string, std::shared_ptr<FileNode>>; // a flattened file tree
    static NodeMap flatten(const FileTree& tree);
    static bool isNodeEqual(const FileNode& lhs,
                            const FileNode& rhs);
    static std::string makeRelativePath(const std::filesystem::path& root,
                                        const std::filesystem::path& absolutePath);
};

} // namespace backup::filesystem