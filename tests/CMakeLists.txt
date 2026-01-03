set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)

ADD_SUBDIRECTORY(googletest EXCLUDE_FROM_ALL)
AUX_SOURCE_DIRECTORY(. SOURCE)
ADD_EXECUTABLE(backup_tests ${SOURCE})

TARGET_INCLUDE_DIRECTORIES(backup_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include
)

TARGET_LINK_LIBRARIES(backup_tests backup_core gtest_main)

ADD_TEST(NAME backup_tests COMMAND backup_tests)