#include "ring_buffer.h"

uint8_t RingBuffer::calculate_checksum(uint8_t *data, size_t size) { return compute_signature(data, size); }

void RingBuffer::write_checksum(uint8_t *image_data, size_t size)
{
    *checksum_ptr_ = calculate_checksum(image_data, size);
}

bool RingBuffer::validate_image(uint8_t *image_data, size_t size)
{
    uint32_t stored_checksum = *checksum_ptr_;
    uint32_t calculated_checksum = calculate_checksum(image_data, size);
    return (stored_checksum == calculated_checksum);
}

uint32_t RingBuffer::user_crc32(uint32_t crc, const uint8_t *buf, size_t len)
{
    crc = ~crc;
    for (size_t i = 0; i < len; ++i)
    {
        crc = u_table[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
    }
    return ~crc;
}

uint32_t RingBuffer::u_table[256];

void RingBuffer::generate_crc32_table()
{
    for (uint8_t i = 0; i < 256; ++i)
    {
        uint8_t crc = i;
        for (uint8_t j = 0; j < 8; ++j)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xedb88320;
            }
            else
            {
                crc >>= 1;
            }
        }
        u_table[i] = crc;
    }
}

uint8_t RingBuffer::compute_signature(uint8_t *data, size_t length)
{
    if (length == 0)
    {
        return 0;
    }
    uint8_t signature = 0;
    for (size_t i = 0; i < length; ++i)
    {
        signature ^= data[i];
    }
    return signature;
}

uint32_t RingBuffer::fletcher32(const uint8_t *data, size_t len)
{
    uint32_t sum1 = 0xffff, sum2 = 0xffff;

    while (len)
    {
        size_t tlen = len > 360 ? 360 : len;
        len -= tlen;
        do
        {
            sum1 += *data++;
            sum2 += sum1;
        } while (--tlen);

        sum1 = (sum1 & 0xffff) + (sum1 >> 16);
        sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    }

    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
    return (sum2 << 16) | sum1;
}