#include "FileNode.h"
#include <cassert>

namespace backup::filesystem {

FileNode::FileNode(std::string name,
                   std::string relativePath,
                   FileType type,
                   uintmax_t size,
                   FileTime mtime)
    : name_(std::move(name)),
      relativePath_(std::move(relativePath)),
      type_(type),
      size_(size),
      mtime_(mtime) {}

bool FileNode::isFile() const noexcept {
    return type_ == FileType::File;
}

bool FileNode::isDirectory() const noexcept {
    return type_ == FileType::Directory;
}

const std::string& FileNode::getName() const noexcept {
    return name_;
}

const std::string& FileNode::getRelativePath() const noexcept {
    return relativePath_;
}

uintmax_t FileNode::getSize() const {
    assert(isFile());
    return size_;
}

FileNode::FileTime FileNode::getMTime() const noexcept {
    return mtime_;
}

void FileNode::addChild(std::shared_ptr<FileNode> child) {
    children_.push_back(std::move(child));
}

const std::vector<std::shared_ptr<FileNode>>&
FileNode::getChildren() const noexcept {
    return children_;
}

} // namespace backup::filesystem
