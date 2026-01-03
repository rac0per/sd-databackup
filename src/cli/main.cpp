#include <iostream>
#include <filesystem>
#include <fstream>
#include "backup/BackupManager.h"
#include "backup/BackupMetadata.h"
#include "filesystem/FileTree.h"

using backup::core::BackupManager;
using backup::core::BackupMetadata;
namespace fs = std::filesystem;

void setupTestData() {
    // Create directories
    fs::create_directories("test_data/source/dir1");
    fs::create_directories("test_data/source/dir2");
    fs::create_directories("test_data/backup");

    // Create files
    std::ofstream("test_data/source/file1.txt") << "Hello World\n";
    std::ofstream("test_data/source/dir1/file2.txt") << "Content of file2\n";
    std::ofstream("test_data/source/dir2/file3.txt") << "Content of file3\n";
}

void modifyTestData() {
    // Modify a file
    std::ofstream("test_data/source/file1.txt") << "Hello World Modified\n";
    // Add a new file
    std::ofstream("test_data/source/newfile.txt") << "New file content\n";
    // Remove a file
    fs::remove("test_data/source/dir2/file3.txt");
}

int main() {
    // Setup test data
    setupTestData();

    std::cout << "=== First Backup ===\n";
    {
        BackupManager::BackupConfig config;
        config.sourceRoot = "test_data/source";
        config.backupRoot = "test_data/backup";
        config.deleteRemoved = true;
        config.dryRun = false;  // Actually execute first backup

        BackupManager manager(config);

        std::cout << "Scan...\n";
        manager.scan();

        std::cout << "Build Plan...\n";
        auto plan = manager.buildPlan();

        for (const auto& action : plan) {
            std::cout << "Action: ";
            switch (action.type) {
            case BackupManager::ActionType::CreateDirectory:
                std::cout << "CreateDirectory ";
                break;
            case BackupManager::ActionType::CopyFile:
                std::cout << "CopyFile ";
                break;
            case BackupManager::ActionType::UpdateFile:
                std::cout << "UpdateFile ";
                break;
            case BackupManager::ActionType::RemovePath:
                std::cout << "RemovePath ";
                break;
            }
            std::cout << " -> " << action.targetPath << "\n";
        }

        std::cout << "Execute...\n";
        manager.executePlan(plan);
    }

    // Modify data
    modifyTestData();

    std::cout << "\n=== Incremental Backup ===\n";
    {
        BackupManager::BackupConfig config;
        config.sourceRoot = "test_data/source";
        config.backupRoot = "test_data/backup";
        config.deleteRemoved = true;
        config.dryRun = true;

        BackupManager manager(config);

        std::cout << "Scan...\n";
        manager.scan();

        std::cout << "Build Plan...\n";
        auto plan = manager.buildPlan();

        for (const auto& action : plan) {
            std::cout << "Action: ";
            switch (action.type) {
            case BackupManager::ActionType::CreateDirectory:
                std::cout << "CreateDirectory ";
                break;
            case BackupManager::ActionType::CopyFile:
                std::cout << "CopyFile ";
                break;
            case BackupManager::ActionType::UpdateFile:
                std::cout << "UpdateFile ";
                break;
            case BackupManager::ActionType::RemovePath:
                std::cout << "RemovePath ";
                break;
            }
            std::cout << " -> " << action.targetPath << "\n";
        }

        std::cout << "Execute (dry run)...\n";
        manager.executePlan(plan);
    }

    std::cout << "\n=== Metadata Test ===\n";
    {
        // Create a FileTree for source
        backup::filesystem::FileTree sourceTree("test_data/source");
        sourceTree.build();

        // Write metadata
        std::cout << "Writing metadata...\n";
        BackupMetadata::writeMetadata(sourceTree, "test_data/backup");

        // Read metadata
        std::cout << "Reading metadata...\n";
        auto info = BackupMetadata::readMetadata("test_data/backup");

        std::cout << "Tool: " << info.tool << "\n";
        std::cout << "Created: " << info.createdUTC << "\n";
        std::cout << "Source Root: " << info.sourceRoot << "\n";
        std::cout << "Files:\n";
        for (const auto& file : info.files) {
            std::cout << "  " << (file.isDirectory ? "D" : "F") << " " << file.relativePath
                      << " size=" << file.size << " mtime=" << file.mtimeNs << "\n";
        }
    }

    std::cout << "\nTest completed successfully!\n";
    return 0;
}
