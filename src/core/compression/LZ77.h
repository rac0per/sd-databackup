#pragma once
#include <vector>
#include <tuple>
#include <cstdint>
#include <cstddef>
namespace backup::core::compression
{
    class LZ77
    {
    private:
        static constexpr size_t WINDOW_SIZE = 4095;  
        static constexpr size_t LOOKAHEAD_SIZE = 15; 
        std::tuple<size_t, size_t, uint8_t> findLongestMatch(const std::vector<uint8_t> &data, size_t currentPos);
    public:
        std::vector<uint8_t> compress(const std::vector<uint8_t> &data);
        std::vector<uint8_t> decompress(const std::vector<uint8_t> &data, size_t originalSize);
    };
} 