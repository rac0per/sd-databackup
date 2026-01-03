#include "LZ77.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
namespace backup::core::compression
{
    std::tuple<size_t, size_t, uint8_t> LZ77::findLongestMatch(const std::vector<uint8_t> &data, size_t currentPos)
    {
        size_t bestOffset = 0;
        size_t bestLength = 0;
        uint8_t nextByte = data[currentPos];
        size_t windowStart = (currentPos > WINDOW_SIZE) ? (currentPos - WINDOW_SIZE) : 0;
        size_t lookaheadEnd = std::min(currentPos + LOOKAHEAD_SIZE, data.size());
        for (size_t i = windowStart; i < currentPos; ++i)
        {
            size_t length = 0;
            while (currentPos + length < lookaheadEnd && data[i + length] == data[currentPos + length])
            {
                ++length;
            }
            if (length > bestLength)
            {
                bestLength = length;
                bestOffset = currentPos - i;
                if (currentPos + length < data.size())
                {
                    nextByte = data[currentPos + length];
                }
                else
                {
                    nextByte = 0;
                }
                if (bestLength == LOOKAHEAD_SIZE)
                {
                    break;
                }
            }
        }
        return std::make_tuple(bestOffset, bestLength, nextByte);
    }
    std::vector<uint8_t> LZ77::compress(const std::vector<uint8_t> &data)
    {
        std::vector<uint8_t> compressedData;
        size_t currentPos = 0;
        while (currentPos < data.size())
        {
            auto [offset, length, nextByte] = findLongestMatch(data, currentPos);
            uint16_t code = (offset << 4) | (length & 0x0F);
            compressedData.push_back(static_cast<uint8_t>((code >> 8) & 0xFF));
            compressedData.push_back(static_cast<uint8_t>(code & 0xFF));
            compressedData.push_back(nextByte);
            currentPos += (length + 1);
        }
        return compressedData;
    }
    std::vector<uint8_t> LZ77::decompress(const std::vector<uint8_t> &data, size_t originalSize)
    {
        std::vector<uint8_t> decompressedData;
        size_t currentPos = 0;
        while (currentPos + 2 < data.size())
        {
            uint16_t code = (static_cast<uint16_t>(data[currentPos]) << 8) | data[currentPos + 1];
            size_t offset = (code >> 4) & 0xFFF; 
            size_t length = code & 0x0F;         
            uint8_t nextByte = data[currentPos + 2];
            if (length > 0 && offset <= decompressedData.size())
            {
                size_t startPos = decompressedData.size() - offset;
                for (size_t i = 0; i < length; ++i)
                {
                    if (startPos + i < decompressedData.size())
                    {
                        decompressedData.push_back(decompressedData[startPos + i]);
                    }
                    else
                    {
                        break;
                    }
                }
            }
            decompressedData.push_back(nextByte);
            currentPos += 3;
        }
        if (decompressedData.size() > originalSize)
        {
            decompressedData.resize(originalSize);
        }
        return decompressedData;
    }
} 