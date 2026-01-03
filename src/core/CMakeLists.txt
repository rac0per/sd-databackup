if(WIN32)
    # 我懒得开虚拟机就直接指定OpenSSL路径了，windows用MinGW Makefiles的话find_package(OpenSSL REQUIRED)会报错
    set(OPENSSL_INCLUDE_DIR "D:/Project applications/OpenSSL-Win64/include")
    set(OPENSSL_CRYPTO_LIBRARY "D:/Project applications/OpenSSL-Win64/lib/VC/x64/MDd/libcrypto.lib")
    set(OPENSSL_SSL_LIBRARY "D:/Project applications/OpenSSL-Win64/lib/VC/x64/MDd/libssl.lib")
    set(OPENSSL_FOUND TRUE)
else()
    # Linux平台自动查找OpenSSL
    find_package(OpenSSL REQUIRED)
endif()

add_library(backup_core
    backup/BackupManager.cpp
    backup/BackupMetadata.cpp
    compression/Compression.cpp
    compression/Huffman.cpp
    compression/LZ77.cpp
    encryption/Encryption.cpp
    encryption/AES.cpp
    encryption/NoneEncryption.cpp
    filesystem/FileTreeDiff.cpp
    filesystem/FileNode.cpp
    filesystem/FileTree.cpp
    util/TimeUtils.cpp
)

target_include_directories(backup_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${OPENSSL_INCLUDE_DIR}
)

if(WIN32)
    target_link_libraries(backup_core PUBLIC
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
    )
else()
    target_link_libraries(backup_core PUBLIC
        OpenSSL::Crypto
        OpenSSL::SSL
    )
endif()
