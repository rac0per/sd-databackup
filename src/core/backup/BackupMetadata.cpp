#include "BackupMetadata.h"
#include "util/TimeUtils.h"

#include <fstream>
#include <stdexcept>

namespace backup::core {

void BackupMetadata::writeMetadata(const filesystem::FileTree& sourceTree,
                                   const std::filesystem::path& backupRoot) {
    const auto metaPath = backupRoot / ".backupmeta";
    std::ofstream out(metaPath, std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open metadata file for writing");
    }

    out << "tool=sd-databackup\n";
    out << "created=" << util::currentTimeUTC() << "\n";
    out << "source_root=" << sourceTree.getRootPath().string() << "\n";

    out << "[filelist]\n";

    sourceTree.traverseDFS([&](const filesystem::FileNode& node) {
        const std::string& relPath = node.getRelativePath();
        // skip root
        if (relPath == ".") {
            return;
        }

        if (node.isDirectory()) {
            out << "D|"
                << relPath
                << "|0|0\n";
        } else {
            out << "F|"
                << relPath
                << "|"
                << node.getSize()
                << "|"
                << util::fileTimeToInt64(node.getMTime())
                << "\n";
        }
    });

    out.flush();
    if (!out.good()) {
        throw std::runtime_error("Failed to write metadata file");
    }
}

} // namespace backup::core
