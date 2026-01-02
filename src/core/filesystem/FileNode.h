#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

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
    using FileTime = std::filesystem::file_time_type;

    FileNode(std::string name,          // name of the file or directory
             std::string relativePath,  // path relative to the root of the file tree, never empty, root = "."
             FileType type,
             uintmax_t size = 0,
             FileTime mtime = FileTime{});
    bool isFile() const noexcept;
    bool isDirectory() const noexcept;
    
    const std::string& getName() const noexcept;
    const std::string& getRelativePath() const noexcept;
    uintmax_t getSize() const; // valid only for file
    FileTime getMTime() const noexcept;

    void addChild(std::shared_ptr<FileNode> child);
    const std::vector<std::shared_ptr<FileNode>>& getChildren() const noexcept;

private:
    std::string name_;
    std::string relativePath_;
    FileType type_;
    uintmax_t size_;       
    FileTime mtime_;


    std::vector<std::shared_ptr<FileNode>> children_;
};
} // namespace backup::filesystem
