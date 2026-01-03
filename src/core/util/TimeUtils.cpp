#include "TimeUtils.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace backup::util {

std::string currentTimeUTC() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

int64_t fileTimeToInt64(std::filesystem::file_time_type ft) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        ft.time_since_epoch()
    ).count();
}

std::filesystem::file_time_type int64ToFileTime(int64_t ns) {
    return std::filesystem::file_time_type{
        std::chrono::nanoseconds(ns)
    };
}

} // namespace backup::util
