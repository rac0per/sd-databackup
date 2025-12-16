add_executable(backup_system
    main.cpp
)

target_include_directories(backup_system
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/core
)

target_link_libraries(backup_system
    PRIVATE backup_core
)
