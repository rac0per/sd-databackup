#pragma once

#include <string>
#include <filesystem>
#include <cstdint>

namespace backup::util {

/**
 * @return 当前 UTC 时间，ISO 8601 格式，如：
 *         2026-01-03T08:21:34Z
 */
std::string currentTimeUTC();

/**
 * filesystem::file_time_type → int64 (nanoseconds since epoch)
 */
int64_t fileTimeToInt64(std::filesystem::file_time_type ft);

/**
 * int64 (nanoseconds since epoch) → filesystem::file_time_type
 * 为 restore 准备
 */
std::filesystem::file_time_type int64ToFileTime(int64_t ns);

} // namespace backup::util
