#pragma once

#include <stdint.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <string>

#include "CImg.h" // temp, for debugging


class Image {
    protected:
        uint8_t* const m_data;

    public:
        const uint16_t width;
        const uint16_t height;
        const uint32_t size;

        Image(const uint16_t static_width, const uint16_t static_height, uint8_t* const ptr_data) noexcept :
            m_data(ptr_data),
            width(static_width),
            height(static_height),
            size(static_cast<uint32_t>(static_width) * static_height)
            {}

        // shallow copy ctor
        Image(const Image& other) noexcept = default;

        // move ctor = delete
        Image(Image&& other) noexcept = delete;

        // deep copy assignment
        Image& operator=(const Image& other) {
            if (this != &other) {
                assert(width == other.width && height == other.height &&
                       "Image cannot be copy-assigned from differently sized other image");

                std::memcpy(m_data, other.m_data, size);
            }

            return *this;
        }

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
};

template<uint16_t static_width, uint16_t static_height>
class StaticImage : public Image {
    private:
        uint8_t m_internal_buff[static_width * static_height];

    public:
        StaticImage() noexcept :
            Image(static_width, static_height, &m_internal_buff[0])
            {}

        StaticImage(uint8_t* const ext_img_buff) :
            StaticImage()
            {
                std::memcpy(m_data, ext_img_buff, size);
            }
        
        // copy/polymorphic ctor
        StaticImage(const Image& other) :
            StaticImage()
            {
                assert(width == other.width && height == other.height &&
                       "StaticImage cannot be copy-constructed from differently sized other image");
                
                std::memcpy(m_data, other.data(), size);
            }
        
        // move ctor = delete
        StaticImage(StaticImage&& other) noexcept = delete;

        // copy/polymorphic assignment
        StaticImage& operator=(const Image& other) {
            if (this != &other) {
                assert(width == other.width && height == other.height &&
                       "Image cannot be copy-assigned from differently sized other image");

                std::memcpy(m_data, other.data(), size);
            }

            return *this;
        }

        // move assignment = delete
        StaticImage& operator=(StaticImage&& other) noexcept = delete;

};
