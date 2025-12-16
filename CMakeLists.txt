cmake_minimum_required(VERSION 3.16)

project(BackupSystem
    VERSION 1.0
    DESCRIPTION "Data Backup Software"
    LANGUAGES CXX
)

# ===== 基本配置 =====
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 开启测试
enable_testing()

# ===== 子目录 =====
add_subdirectory(src)
add_subdirectory(tests)
