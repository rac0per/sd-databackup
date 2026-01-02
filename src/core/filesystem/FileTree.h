#pragma once
#include <filesystem>
#include <memory>
#include <functional>
#include "FileNode.h"

namespace backup::filesystem {

class FileTree {
public:
    explicit FileTree(const std::filesystem::path& rootPath);

    FileTree(const FileTree&) = delete;
    FileTree& operator=(const FileTree&) = delete;

    void build();

    std::shared_ptr<FileNode> getRoot() const noexcept;
    const std::filesystem::path& getRootPath() const noexcept;

    void traverseDFS(const std::function<void(const FileNode&)>& visitor) const;

private:
    std::filesystem::path rootPath_;
    std::shared_ptr<FileNode> root_;

    void buildRecursive(const std::filesystem::path& absPath,
                        const std::shared_ptr<FileNode>& parent);

    void traverseDFSRecursive(const std::shared_ptr<FileNode>& node,
                              const std::function<void(const FileNode&)>& visitor) const;
};

} // namespace backup::filesystem
