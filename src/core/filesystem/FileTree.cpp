#include "FileTree.h"
#include <filesystem>
#include <stdexcept>

namespace backup::filesystem {
namespace fs = std::filesystem;


FileTree::FileTree(const fs::path& rootPath)
    : rootPath_(rootPath){}

void FileTree::build() {
    if(!fs::exists(rootPath_)) {
        throw std::runtime_error("Root path must be an existing directory");
    }
    if(!fs::is_directory(rootPath_)) {
        throw std::runtime_error("Root path must be a directory");
    }

    root_ = std::make_shared<FileNode>(rootPath_.filename().string(),
                                        "",
                                        FileType::Directory);
    buildRecursive(rootPath_, root_);
}
void FileTree::buildRecursive(const fs::path& currentPath,
                        std::shared_ptr<FileNode>& parent) {
    for (const auto& entry : fs::directory_iterator(currentPath)) {
        const fs::path& path = entry.path();
        std::string name = path.filename().string();

        // compute relative path (relative to rootPath_)
        fs::path relative = fs::relative(path, rootPath_);
        std::string relativePath = relative.string();

        if (entry.is_directory()) {
            auto dirNode = std::make_shared<FileNode>(
                name,
                relativePath,
                FileType::Directory
            );

            parent->addChild(dirNode);
            buildRecursive(path, dirNode);
        } else if (entry.is_regular_file()) {
            auto fileNode = std::make_shared<FileNode>(
                name,
                relativePath,
                FileType::File
            );

            parent->addChild(fileNode);
        }
    }         
}               

std::shared_ptr<FileNode> FileTree::getRoot() const {
    return root_;
}

void FileTree::traverseDFS(const std::function<void(const FileNode&)>& visitor) const {
    if (!root_) {
        return;
    }
    traverseDFSRecursive(root_, visitor);
}

void FileTree::traverseDFSRecursive(const std::shared_ptr<FileNode>& node,
                          const std::function<void(const FileNode&)>& visitor) const {
    visitor(*node);
    if (node->isDirectory()) {
        for (const auto& child : node->getChildren()) {
            traverseDFSRecursive(child, visitor);
        }
    }
}



std::shared_ptr<FileNode> FileTree::findNode(const std::string& relativePath) const {
    if (!root_) {
        return nullptr;
    }
    return findNodeDFS(root_, relativePath);

}

std::shared_ptr<FileNode> FileTree::findNodeDFS(const std::shared_ptr<FileNode>& node,
                                        const std::string& relativePath) const {
    if (!node) {
        return nullptr;
    }
    if (node->getRelativePath() == relativePath) {
        return node;
    }
    for (const auto& child : node->getChildren()) {
        auto found = findNodeDFS(child, relativePath);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

const std::filesystem::path& FileTree::getRootPath() const {
    return rootPath_;
}

} //namespace backup::filesystem