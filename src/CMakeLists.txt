add_subdirectory(core)
add_subdirectory(cli)
add_executable(filetree_test
    test_filetree.cpp
)

target_include_directories(filetree_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_link_libraries(filetree_test
    PRIVATE backup_core
)