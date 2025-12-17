#pragma once

#include <string>
#include <vector>
#include <memory>

namespace backup::filesystem {

enum class FileType {
    File,
    Directory
};

class FileNode {
public:
    FileNode(std::string name,
             std::string relativePath,
             FileType type);
    bool isFile() const;
    bool isDirectory() const;
    
    const std::string& getName() const;
    const std::string& getRelativePath() const;

    void addChild(std::shared_ptr<FileNode> child);
    const std::vector<std::shared_ptr<FileNode>>& getChildren() const;

private:
    std::string name_;
    std::string relativePath_;
    FileType type_;

    std::vector<std::shared_ptr<FileNode>> children_;
};
} // namespace backup::filesystem
