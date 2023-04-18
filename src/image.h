#pragma once

#include <stdint.h>


struct Image {
    private:
        uint8_t* const m_data;
        const uint32_t m_data_len;

    public:
        const uint16_t width;
        const uint16_t height;
        uint8_t pad_value;

        Image(uint8_t* const img_buff, const uint16_t width, const uint16_t height) noexcept;

        uint8_t& operator[](uint32_t idx) const;
        uint8_t& at(const uint16_t row, const uint16_t col) const noexcept;
        uint8_t at(const int32_t row, const int32_t col, uint8_t pad_value) const noexcept;

        Image& threshold(const uint8_t threshold) noexcept;
        Image& absdiff(const Image& other);
};
