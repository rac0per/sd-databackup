#include "FileTreeDiff.h"
#include <algorithm>

namespace backup::filesystem {

std::vector<FileChange>
FileTreeDiff::diff(const FileTree& oldTree, const FileTree& newTree) {
    auto oldMap = flatten(oldTree);
    auto newMap = flatten(newTree);

    std::vector<FileChange> changes;

    // Removed / Modified
    for (const auto& kv : oldMap) {
        const auto& path = kv.first;
        const auto& oldNode = kv.second;

        auto it = newMap.find(path);
        if (it == newMap.end()) {
            changes.push_back(
                {ChangeType::Removed, path, oldNode, nullptr});
        } else if (oldNode->isFile() &&
                   !isSameFile(*oldNode, *it->second)) {
            changes.push_back(
                {ChangeType::Modified, path, oldNode, it->second});
        }
    }

    // Added
    for (const auto& kv : newMap) {
        const auto& path = kv.first;
        if (oldMap.find(path) == oldMap.end()) {
            changes.push_back(
                {ChangeType::Added, path, nullptr, kv.second});
        }
    }

    std::sort(changes.begin(), changes.end(),
              [](const FileChange& a, const FileChange& b) {
                  return a.relativePath < b.relativePath;
              });

    return changes;
}

FileTreeDiff::NodeMap
FileTreeDiff::flatten(const FileTree& tree) {
    NodeMap map;

    auto root = tree.getRoot();
    if (!root) return map;

    std::function<void(const std::shared_ptr<FileNode>&)> dfs;
    dfs = [&](const std::shared_ptr<FileNode>& node) {
        if (node->getRelativePath() != ".") {
            map.emplace(node->getRelativePath(), node);
        }
        if (node->isDirectory()) {
            for (const auto& c : node->getChildren()) {
                dfs(c);
            }
        }
    };

    dfs(root);
    return map;
}

bool FileTreeDiff::isSameFile(const FileNode& a, const FileNode& b) {
    return a.getSize() == b.getSize() &&
           a.getMTime() == b.getMTime();
}

} // namespace backup::filesystem
