#pragma once

#include <stdint.h>


class Image {

    private:

        uint8_t* const m_data;

    public:

        const uint16_t width {};
        const uint16_t height {};

        Image(uint8_t* const ptr_data, const uint16_t width, const uint16_t height) noexcept :
            m_data(ptr_data),
            width(width),
            height(height)
            {}

        Image(const Image& other) = delete;
        Image& operator=(const Image& other) = delete;
        Image(Image&& other) = delete;
        Image& operator=(Image&& other) = delete;
        virtual ~Image() = default;

        const uint8_t* data() const noexcept {

            return m_data;
        }

        uint8_t* data() noexcept {

            return m_data;
        }

        const uint8_t& at(const uint16_t row, const uint16_t col) const noexcept {

            return m_data[row * width + col];
        }

        uint8_t& at(const uint16_t row, const uint16_t col) noexcept {

            return m_data[row * width + col];
        }

        uint8_t at(const int32_t row, const int32_t col, const uint8_t pad_value) const noexcept {

            return (row >= 0 && col >= 0 && row < height && col < width) ?
                m_data[row * width + col] : pad_value;
        }
};
