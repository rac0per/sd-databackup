add_library(backup_core
    backup/BackupManager.cpp
    compression/Compression.cpp
    compression/Huffman.cpp
    compression/LZ77.cpp
    encryption/Encryption.cpp
    encryption/MD5.cpp
    filesystem/FileTreeDiff.cpp
    filesystem/FileNode.cpp
    filesystem/FileTree.cpp
)

target_include_directories(backup_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
