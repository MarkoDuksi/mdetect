#pragma once

#include <stdint.h>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <string>

#include "CImg.h"


class Image {
    protected:
        const uint16_t m_width;
        const uint16_t m_height;
        const uint32_t m_size;
        uint8_t* const m_data;

    public:
        Image(const uint16_t static_width, const uint16_t static_height, uint8_t* const ptr_data) noexcept :
            m_width(static_width),
            m_height(static_height),
            m_size(static_cast<uint32_t>(static_width) * static_height),
            m_data(ptr_data)
            {}

        // copy assignment
        Image& operator=(const Image& other) {
            if (this != &other) {
                assert(m_width == other.m_width && m_height == other.m_height &&
                       "Image cannot be copy-assigned from differently sized other image");

                std::memcpy(m_data, other.m_data, m_size);
            }

            return *this;
        }

        // reenable shallow copy
        Image(const Image& other) noexcept = default;

        uint16_t width() const noexcept {
            return m_width;
        }

        uint16_t height() const noexcept {
            return m_height;
        }

        uint32_t size() const noexcept {
            return m_size;
        }

        uint8_t* data() const noexcept {
            return m_data;
        }

        uint8_t& at(const uint16_t row, const uint16_t col) const {
            return m_data[static_cast<uint32_t>(row) * m_width + col];
        }

        uint8_t at(const int32_t row, const int32_t col, const uint8_t pad_value) const noexcept {
            return row >= 0 && col >= 0 && row < m_height && col < m_width ?
                m_data[static_cast<uint32_t>(row) * m_width + col] : pad_value;
        }

       void save(const char *filename) const {
            cimg_library::CImg<uint8_t> img_grey;
            img_grey.assign(m_data, m_width, m_height);
            img_grey.rotate(-90).save(filename);
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
                std::memcpy(m_data, ext_img_buff, m_size);
            }
        
        // copy/polymorphic ctor
        StaticImage(const Image& other) :
            StaticImage()
            {
                assert(m_width == other.width() && m_height == other.height() &&
                       "StaticImage cannot be copy-constructed from differently sized other image");
                
                std::memcpy(m_data, other.data(), m_size);
            }
        
        // copy/polymorphic assignment
        StaticImage& operator=(const Image& other) {
            if (this != &other) {
                assert(m_width == other.width() && m_height == other.height() &&
                       "Image cannot be copy-assigned from differently sized other image");

                std::memcpy(m_data, other.data(), m_size);
            }

            return *this;
        }
};
