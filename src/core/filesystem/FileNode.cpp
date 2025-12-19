#include "filesystem/FileNode.h"

namespace backup::filesystem {
FileNode::FileNode(std::string name, 
                   std::string relativePath,
                   FileType type)
    :name_(std::move(name)),
     relativePath_(std::move(relativePath)),
     type_(type) {}

bool FileNode::isFile() const {
    return type_ == FileType::File;
}

bool FileNode::isDirectory() const {
    return type_ == FileType::Directory;
}

const std::string& FileNode::getName() const {
    return name_;
}

const std::string& FileNode::getRelativePath() const {
    return relativePath_;
}

void FileNode::addChild(std::shared_ptr<FileNode> child) {
    if (isDirectory()) {
        children_.push_back(std::move(child));
    }
}

const std::vector<std::shared_ptr<FileNode>>& FileNode::getChildren() const {
    return children_;
}
} // namespace backup::filesystem