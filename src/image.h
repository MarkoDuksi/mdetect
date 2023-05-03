#pragma once

#include <stdint.h>
#include <string>


struct Image {
        private:
                uint8_t* m_data;
                uint16_t m_width;
                uint16_t m_height;
                uint32_t m_size;

        public:
                Image(uint8_t* const img_buff, const uint16_t width, const uint16_t height) noexcept;

                Image(const Image& other) = delete;
                Image& operator=(const Image& other);

                // redefine *shallow" move ctor
                Image(Image&& other) noexcept = default;

                uint16_t width() const noexcept;
                uint16_t height() const noexcept;
                uint32_t size() const noexcept;

                // redefine *shallow" move assignment op
                Image& operator=(Image&& other) noexcept = default;

                uint8_t& at(const uint16_t row, const uint16_t col) const;
                uint8_t at(const int32_t row, const int32_t col, uint8_t pad_value) const noexcept;

                Image& threshold(const uint8_t threshold) noexcept;
                Image& absdiff(const Image& other);

                void save(const char* filename) const;
};
