#pragma once

#include <filesystem>
#include "filesystem/FileTree.h"


namespace backup::core{
class BackupMetadata {
public:
    static void writeMetadata(
        const filesystem::FileTree& sourceTree,
        const std::filesystem::path& backupRoot);
};
} // namespace backup::core