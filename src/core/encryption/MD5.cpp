#include "MD5.h"
#include <cstring>
namespace backup::core::encryption
{
    MD5::MD5()
    {
        reset();
    }
    void MD5::reset()
    {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        count[0] = 0;
        count[1] = 0;
        memset(buffer, 0, 64);
    }
    void MD5::update(const uint8_t *input, size_t length)
    {
        size_t i, index, partLen;
        index = (count[0] >> 3) & 0x3F;
        count[0] += length << 3;
        if (count[0] < (length << 3))
        {
            count[1]++;
        }
        count[1] += length >> 29;
        partLen = 64 - index;
        if (length >= partLen)
        {
            memcpy(&buffer[index], input, partLen);
            transform(buffer);
            for (i = partLen; i + 63 < length; i += 64)
            {
                transform(&input[i]);
            }
            index = 0;
        }
        else
        {
            i = 0;
        }
        memcpy(&buffer[index], &input[i], length - i);
    }
    void MD5::update(const std::string &input)
    {
        update(reinterpret_cast<const uint8_t *>(input.c_str()), input.size());
    }
    void MD5::final(uint8_t digest[16])
    {
        uint8_t bits[8];
        size_t index, padLen;
        encode(bits, count, 8);
        index = (count[0] >> 3) & 0x3F;
        padLen = (index < 56) ? (56 - index) : (120 - index);
        update(padding, padLen);
        update(bits, 8);
        encode(digest, state, 16);
        reset();
    }
    std::string MD5::final()
    {
        uint8_t digest[16];
        final(digest);
        return toHexString(digest, 16);
    }
    void MD5::transform(const uint8_t block[64])
    {
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];
        decode(x, block, 64);
        FF(a, b, c, d, x[0], S11, 0xD76AA478);
        FF(d, a, b, c, x[1], S12, 0xE8C7B756);
        FF(c, d, a, b, x[2], S13, 0x242070DB);
        FF(b, c, d, a, x[3], S14, 0xC1BDCEEE);
        FF(a, b, c, d, x[4], S11, 0xF57C0FAF);
        FF(d, a, b, c, x[5], S12, 0x4787C62A);
        FF(c, d, a, b, x[6], S13, 0xA8304613);
        FF(b, c, d, a, x[7], S14, 0xFD469501);
        FF(a, b, c, d, x[8], S11, 0x698098D8);
        FF(d, a, b, c, x[9], S12, 0x8B44F7AF);
        FF(c, d, a, b, x[10], S13, 0xFFFF5BB1);
        FF(b, c, d, a, x[11], S14, 0x895CD7BE);
        FF(a, b, c, d, x[12], S11, 0x6B901122);
        FF(d, a, b, c, x[13], S12, 0xFD987193);
        FF(c, d, a, b, x[14], S13, 0xA679438E);
        FF(b, c, d, a, x[15], S14, 0x49B40821);
        GG(a, b, c, d, x[1], S21, 0xF61E2562);
        GG(d, a, b, c, x[6], S22, 0xC040B340);
        GG(c, d, a, b, x[11], S23, 0x265E5A51);
        GG(b, c, d, a, x[0], S24, 0xE9B6C7AA);
        GG(a, b, c, d, x[5], S21, 0xD62F105D);
        GG(d, a, b, c, x[10], S22, 0x02441453);
        GG(c, d, a, b, x[15], S23, 0xD8A1E681);
        GG(b, c, d, a, x[4], S24, 0xE7D3FBC8);
        GG(a, b, c, d, x[9], S21, 0x21E1CDE6);
        GG(d, a, b, c, x[14], S22, 0xC33707D6);
        GG(c, d, a, b, x[3], S23, 0xF4D50D87);
        GG(b, c, d, a, x[8], S24, 0x455A14ED);
        GG(a, b, c, d, x[13], S21, 0xA9E3E905);
        GG(d, a, b, c, x[2], S22, 0xFCEFA3F8);
        GG(c, d, a, b, x[7], S23, 0x676F02D9);
        GG(b, c, d, a, x[12], S24, 0x8D2A4C8A);
        HH(a, b, c, d, x[5], S31, 0xFFFA3942);
        HH(d, a, b, c, x[8], S32, 0x8771F681);
        HH(c, d, a, b, x[11], S33, 0x6D9D6122);
        HH(b, c, d, a, x[14], S34, 0xFDE5380C);
        HH(a, b, c, d, x[1], S31, 0xA4BEEA44);
        HH(d, a, b, c, x[4], S32, 0x4BDECFA9);
        HH(c, d, a, b, x[7], S33, 0xF6BB4B60);
        HH(b, c, d, a, x[10], S34, 0xBEBFBC70);
        HH(a, b, c, d, x[13], S31, 0x289B7EC6);
        HH(d, a, b, c, x[0], S32, 0xEAA127FA);
        HH(c, d, a, b, x[3], S33, 0xD4EF3085);
        HH(b, c, d, a, x[6], S34, 0x04881D05);
        HH(a, b, c, d, x[9], S31, 0xD9D4D039);
        HH(d, a, b, c, x[12], S32, 0xE6DB99E5);
        HH(c, d, a, b, x[15], S33, 0x1FA27CF8);
        HH(b, c, d, a, x[2], S34, 0xC4AC5665);
        II(a, b, c, d, x[0], S41, 0xF4292244);
        II(d, a, b, c, x[7], S42, 0x432AFF97);
        II(c, d, a, b, x[14], S43, 0xAB9423A7);
        II(b, c, d, a, x[5], S44, 0xFC93A039);
        II(a, b, c, d, x[12], S41, 0x655B59C3);
        II(d, a, b, c, x[3], S42, 0x8F0CCC92);
        II(c, d, a, b, x[10], S43, 0xFFEFF47D);
        II(b, c, d, a, x[1], S44, 0x85845DD1);
        II(a, b, c, d, x[8], S41, 0x6FA87E4F);
        II(d, a, b, c, x[15], S42, 0xFE2CE6E0);
        II(c, d, a, b, x[6], S43, 0xA3014314);
        II(b, c, d, a, x[13], S44, 0x4E0811A1);
        II(a, b, c, d, x[4], S41, 0xF7537E82);
        II(d, a, b, c, x[11], S42, 0xBD3AF235);
        II(c, d, a, b, x[2], S43, 0x2AD7D2BB);
        II(b, c, d, a, x[9], S44, 0xEB86D391);
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
    }
    void MD5::decode(uint32_t output[], const uint8_t input[], size_t len)
    {
        for (size_t i = 0, j = 0; j < len; i++, j += 4)
        {
            output[i] = (input[j] & 0xFF) |
                        ((input[j + 1] & 0xFF) << 8) |
                        ((input[j + 2] & 0xFF) << 16) |
                        ((input[j + 3] & 0xFF) << 24);
        }
    }
    void MD5::encode(uint8_t output[], const uint32_t input[], size_t len)
    {
        for (size_t i = 0, j = 0; j < len; i++, j += 4)
        {
            output[j] = input[i] & 0xFF;
            output[j + 1] = (input[i] >> 8) & 0xFF;
            output[j + 2] = (input[i] >> 16) & 0xFF;
            output[j + 3] = (input[i] >> 24) & 0xFF;
        }
    }
    std::string MD5::toHexString(const uint8_t *data, size_t length)
    {
        const char hexChars[] = "0123456789abcdef";
        std::string result;
        result.reserve(length * 2);
        for (size_t i = 0; i < length; i++)
        {
            result.push_back(hexChars[(data[i] >> 4) & 0x0F]);
            result.push_back(hexChars[data[i] & 0x0F]);
        }
        return result;
    }
    uint32_t MD5::F(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & y) | (~x & z);
    }
    uint32_t MD5::G(uint32_t x, uint32_t y, uint32_t z)
    {
        return (x & z) | (y & ~z);
    }
    uint32_t MD5::H(uint32_t x, uint32_t y, uint32_t z)
    {
        return x ^ y ^ z;
    }
    uint32_t MD5::I(uint32_t x, uint32_t y, uint32_t z)
    {
        return y ^ (x | ~z);
    }
    uint32_t MD5::ROTATE_LEFT(uint32_t x, int n)
    {
        return (x << n) | (x >> (32 - n));
    }
    void MD5::FF(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac)
    {
        a += F(b, c, d) + x + ac;
        a = ROTATE_LEFT(a, s);
        a += b;
    }
    void MD5::GG(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac)
    {
        a += G(b, c, d) + x + ac;
        a = ROTATE_LEFT(a, s);
        a += b;
    }
    void MD5::HH(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac)
    {
        a += H(b, c, d) + x + ac;
        a = ROTATE_LEFT(a, s);
        a += b;
    }
    void MD5::II(uint32_t &a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, int s, uint32_t ac)
    {
        a += I(b, c, d) + x + ac;
        a = ROTATE_LEFT(a, s);
        a += b;
    }
    const uint8_t MD5::padding[64] = {
        0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
} 