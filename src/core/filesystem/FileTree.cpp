#include "FileTree.h"
#include <stdexcept>

namespace backup::filesystem {
namespace fs = std::filesystem;

FileTree::FileTree(const fs::path& rootPath)
    : rootPath_(rootPath) {}

void FileTree::build() {
    if (!fs::exists(rootPath_) || !fs::is_directory(rootPath_)) {
        throw std::runtime_error("Root path must be an existing directory");
    }

    root_ = std::make_shared<FileNode>(
        rootPath_.filename().string(),
        ".",
        FileType::Directory
    );

    buildRecursive(rootPath_, root_);
}

void FileTree::buildRecursive(const fs::path& absPath,
                              const std::shared_ptr<FileNode>& parent) {
    try {
        for (const auto& entry : fs::directory_iterator(absPath)) {
            const auto& p = entry.path();
            auto rel = fs::relative(p, rootPath_).generic_string();

            if (entry.is_directory()) {
                auto dir = std::make_shared<FileNode>(
                    p.filename().string(),
                    rel,
                    FileType::Directory
                );
                parent->addChild(dir);
                buildRecursive(p, dir);
            } else if (entry.is_regular_file()) {
                auto file = std::make_shared<FileNode>(
                    p.filename().string(),
                    rel,
                    FileType::File,
                    entry.file_size(),
                    entry.last_write_time()
                );
                parent->addChild(file);
            }
        }
    } catch (const fs::filesystem_error&) {
        // skip unreadable directories
    }
}

std::shared_ptr<FileNode> FileTree::getRoot() const noexcept {
    return root_;
}

const fs::path& FileTree::getRootPath() const noexcept {
    return rootPath_;
}

void FileTree::traverseDFS(
    const std::function<void(const FileNode&)>& visitor) const {
    if (root_) {
        traverseDFSRecursive(root_, visitor);
    }
}

void FileTree::traverseDFSRecursive(
    const std::shared_ptr<FileNode>& node,
    const std::function<void(const FileNode&)>& visitor) const {

    visitor(*node);
    if (node->isDirectory()) {
        for (const auto& child : node->getChildren()) {
            traverseDFSRecursive(child, visitor);
        }
    }
}

} // namespace backup::filesystem
