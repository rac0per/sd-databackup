add_library(backup_core
    backup/BackupManager.cpp
    filesystem/FileScanner.cpp
)

target_include_directories(backup_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
