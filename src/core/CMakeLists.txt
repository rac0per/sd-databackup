add_library(backup_core
    backup/BackupManager.cpp
    backup/BackupMetadata.cpp
    filesystem/FileTreeDiff.cpp
    filesystem/FileNode.cpp
    filesystem/FileTree.cpp
    util/TimeUtils.cpp
)

target_include_directories(backup_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
