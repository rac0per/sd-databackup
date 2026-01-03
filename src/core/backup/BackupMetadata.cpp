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

BackupMetadataInfo BackupMetadata::readMetadata(const std::filesystem::path& backupRoot) {
    const auto metaPath = backupRoot / ".backupmeta";
    std::ifstream in(metaPath);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open metadata file for reading");
    }

    BackupMetadataInfo info;
    std::string line;

    enum class Section { None, FileList };
    Section currentSection = Section::None;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        // section header
        if (line == "[filelist]") {
            currentSection = Section::FileList;
            continue;
        }

        if (currentSection == Section::None) {
            // key=value
            auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "tool") {
                info.tool = value;
            } else if (key == "created") {
                info.createdUTC = value;
            } else if (key == "source_root") {
                info.sourceRoot = value;
            }
        } else if (currentSection == Section::FileList) {
            // parse file entry
            // format: F|relPath|size|mtime or D|relPath|0|0
            std::istringstream ss(line);
            std::string token;
            std::vector<std::string> parts;
            while (std::getline(ss, token, '|')) {
                parts.push_back(token);
            }
            if (parts.size() != 4) continue;

            BackupFileEntry entry;
            entry.isDirectory = (parts[0] == "D");
            entry.relativePath = parts[1];
            entry.size = std::stoull(parts[2]);
            entry.mtimeNs = std::stoll(parts[3]);
            info.files.push_back(std::move(entry));
        }
    }

    return info;

}

} // namespace backup::core
