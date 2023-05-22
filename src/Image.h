#pragma once

#include <stdint.h>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <string>

#include "CImg.h" // temp, for debugging


class Image {

    public:

        const uint16_t width {};
        const uint16_t height {};
        const uint32_t size {};

        Image(const uint16_t width, const uint16_t height, uint8_t* const ptr_data) noexcept :
            width(width),
            height(height),
            size(static_cast<uint32_t>(width) * height),
            m_data(ptr_data)
            {}

        // shallow copy ctor = default
        Image(const Image& other) noexcept = default;

        // deep copy assignment
        Image& operator=(const Image& other) {

            if (this != &other) {

                assert(width == other.width && height == other.height &&
                       "Image cannot be copy-assigned from differently sized other image");

                std::memcpy(m_data, other.m_data, size);
            }

            return *this;
        }

        // move ctor = delete
        Image(Image&& other) noexcept = delete;

        // move assignment = delete
        Image& operator=(Image&& other) noexcept = delete;

        uint8_t* data() const noexcept {

            return m_data;
        }

        uint8_t& at(const uint16_t row, const uint16_t col) const {

            return m_data[static_cast<uint32_t>(row) * width + col];
        }

        uint8_t at(const int32_t row, const int32_t col, const uint8_t pad_value) const noexcept {

            return (row >= 0 && col >= 0 && row < height && col < width) ?
                m_data[static_cast<uint32_t>(row) * width + col] : pad_value;
        }

       // temp, for debugging
       void save(const char *filename, const int number = -1, const uint digits = 2) const {

            cimg_library::CImg<uint8_t> img_grey;

            img_grey.assign(m_data, width, height);
            img_grey.rotate(-90).save(filename, number, digits);
        }

    protected:

        uint8_t* const m_data;
};

template<uint16_t IMG_WIDTH, uint16_t IMG_HEIGHT>
class StaticImage : public Image {

    public:

        StaticImage() noexcept :
            Image(IMG_WIDTH, IMG_HEIGHT, &m_internal_buff[0])
            {}
        
        // conversion assignment op
        StaticImage& operator=(const Image& other) {
            if (this != &other) {
                assert(width == other.width && height == other.height &&
                       "Image cannot be copy-assigned from differently sized other image.");

                std::memcpy(m_data, other.data(), size);
            }

            return *this;
        }

    private:

        uint8_t m_internal_buff[IMG_WIDTH * IMG_HEIGHT];
};
