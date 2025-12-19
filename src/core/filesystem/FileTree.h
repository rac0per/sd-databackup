#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>

#include "FileNode.h"

namespace backup::filesystem {

class FileTree {
public:
    explicit FileTree(const std::filesystem::path& rootPath);
    
    FileTree(const FileTree&) = delete;             // banned copy constructor
    FileTree& operator=(const FileTree&) = delete;  // banned copy assignment
    FileTree(FileTree&&) = default;                  // default move constructor
    FileTree& operator=(FileTree&&) = default;       // default move assignment

    void build();

    std::shared_ptr<FileNode> getRoot() const;

    void traverseDFS(const std::function<void(const FileNode&)>& visitor) const;

    std::shared_ptr<FileNode> findNode(const std::string& relativePath) const;

    const std::filesystem::path& getRootPath() const;

    private:
    std::shared_ptr<FileNode> root_;
    std::filesystem::path rootPath_;

    void buildRecursive(const std::filesystem::path& currentPath,
                        std::shared_ptr<FileNode>& parent);

    void traverseDFSRecursive(const std::shared_ptr<FileNode>& node,
                              const std::function<void(const FileNode&)>& visitor) const;
    std::shared_ptr<FileNode> findNodeDFS(const std::shared_ptr<FileNode>& node,
                                         const std::string& relativePath) const;
};


} // namespace backup::filesystem