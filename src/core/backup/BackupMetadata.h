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
    std::vector<BackupFileEntry> files;
};


class BackupMetadata {
public:
    static void writeMetadata(const filesystem::FileTree& sourceTree, const std::filesystem::path& backupRoot);

    static BackupMetadataInfo readMetadata(const std::filesystem::path& backupRoot);

};
} // namespace backup::core