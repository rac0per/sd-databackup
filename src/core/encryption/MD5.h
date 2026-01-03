#pragma once
#include <string>
#include <cstdint>
#include <vector>
namespace backup::core::encryption
{
    class MD5
    {
    public:
        MD5();
        void reset();
        void update(const uint8_t *input, size_t length);
        void update(const std::string &input);
        void final(uint8_t digest[16]);
        std::string final();
    private:
        void transform(const uint8_t block[64]);
        static void decode(uint32_t output[], const uint8_t input[], size_t len);
        static void encode(uint8_t output[], const uint32_t input[], size_t len);
        static std::string toHexString(const uint8_t *data, size_t length);
        static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z);
        static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z);
        static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z);
        static inline uint32_t I(uint32_t x, uint32_t y, uint32_t z);
        static inline uint32_t ROTATE_LEFT(uint32_t x, int n);
        static inline void FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac);
        static inline void GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac);
        static inline void HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac);
        static inline void II(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac);
        uint32_t state[4];
        uint32_t count[2];
        uint8_t buffer[64];
        static const uint8_t padding[64];
        static const uint32_t S11 = 7;
        static const uint32_t S12 = 12;
        static const uint32_t S13 = 17;
        static const uint32_t S14 = 22;
        static const uint32_t S21 = 5;
        static const uint32_t S22 = 9;
        static const uint32_t S23 = 14;
        static const uint32_t S24 = 20;
        static const uint32_t S31 = 4;
        static const uint32_t S32 = 11;
        static const uint32_t S33 = 16;
        static const uint32_t S34 = 23;
        static const uint32_t S41 = 6;
        static const uint32_t S42 = 10;
        static const uint32_t S43 = 15;
        static const uint32_t S44 = 21;
    };
} 