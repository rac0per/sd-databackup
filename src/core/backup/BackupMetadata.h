#pragma once

#include <filesystem>
#include "filesystem/FileTree.h"


namespace backup::core{

struct BackupFileEntry {
    std::string relativePath;
    bool isDirectory;
    uintmax_t size = 0;
    int64_t mtimeNs = 0;
};

struct BackupMetadataInfo {
    std::string tool;
    std::string createdUTC;
    std::filesystem::path sourceRoot;
    std::string compressionType;  // 压缩算法类型
    std::string encryptionType;   // 加密算法类型
    std::vector<BackupFileEntry> files;
};


class BackupMetadata {
public:
    static void writeMetadata(const filesystem::FileTree& sourceTree, 
                             const std::filesystem::path& backupRoot,
                             const std::string& compressionType = "none",
                             const std::string& encryptionType = "none");

    static BackupMetadataInfo readMetadata(const std::filesystem::path& backupRoot);

};
} // namespace backup::core