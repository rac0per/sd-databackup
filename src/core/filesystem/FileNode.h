#pragma once

#include <string>
#include <vector>
#include <memory>

namespace backup::filesystem {

enum class FileType {
    File,
    Directory
};


/*
 *   FileNode is a class representing a node in a file system tree, 
 *   which can be either a file or a directory.
 */
 
class FileNode {
public:
    FileNode(std::string name,          // name of the file or directory
             std::string relativePath,  // path relative to the root of the file tree
             FileType type);            // type of the node (file or directory)
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
