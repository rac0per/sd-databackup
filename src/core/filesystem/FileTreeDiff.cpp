#include "FileTreeDiff.h"
#include <algorithm>
namespace backup::filesystem {
using ChangeSet = std::vector<FileChange>;

ChangeSet FileTreeDiff::diff(const FileTree& oldTree, const FileTree& newTree) {
    ChangeSet changes;

    FileTreeDiff::NodeMap oldNodes = flatten(oldTree);
    FileTreeDiff::NodeMap newNodes = flatten(newTree);

    // Check for removed and modified files
    for (const auto& [relativePath, oldNode] : oldNodes) {
        auto newIt = newNodes.find(relativePath);
        if (newIt == newNodes.end()) {
            // Node is removed
            changes.push_back({ChangeType::Removed, relativePath, oldNode, nullptr});
        } else {
            // Node exists in both trees, check for modification
            const auto& newNode = newIt->second;
            if (!isNodeEqual(*oldNode, *newNode)) {
                changes.push_back({ChangeType::Modified, relativePath, oldNode, newNode});
            }
        }
    }

    // Check for added files
    for (const auto& [relativePath, newNode] : newNodes) {
        if (oldNodes.find(relativePath) == oldNodes.end()) {
            // Node is added
            changes.push_back({ChangeType::Added, relativePath, nullptr, newNode});
        }
    }
    std::sort(changes.begin(), changes.end(),
          [](const FileChange& a, const FileChange& b) {
              return a.relativePath < b.relativePath;
          }); // unordered_map is randomly ordered
    return changes;
}

FileTreeDiff::NodeMap FileTreeDiff::flatten(const FileTree& tree) {
    FileTreeDiff::NodeMap nodeMap;

    auto root = tree.getRoot();
    if (!root) {    
        return nodeMap;
    }

    std::function<void(const std::shared_ptr<FileNode>&)> dfs;
    dfs = [&](const std::shared_ptr<FileNode>& node) {
        nodeMap.emplace(node->getRelativePath(), node);
        if (node->isDirectory()) {
            for (const auto& child : node->getChildren()) {
                dfs(child);
            }
        }
    };
    dfs(root);
    return nodeMap;
} 
bool FileTreeDiff::isNodeEqual(const FileNode& lhs, const FileNode& rhs) {
    if (lhs.isFile() != rhs.isFile() || lhs.isDirectory() != rhs.isDirectory()) {
        return false;
    }
    
    if (lhs.getRelativePath() != rhs.getRelativePath()) {
        return false;
    }
    return true;
}
std::string FileTreeDiff::makeRelativePath(
    const std::filesystem::path& root,
    const std::filesystem::path& absolutePath) {

    namespace fs = std::filesystem;

    fs::path rel;

    try {
        rel = fs::relative(absolutePath, root);
    } catch (const fs::filesystem_error&) {
        // root 与 absolutePath 不可比较（不同盘符等）
        return absolutePath.generic_string();
    }

    // fs::relative 可能返回 "."
    if (rel == fs::path(".")) {
        return "";
    }

    return rel.generic_string();
}
}// namespace backup::filesystem