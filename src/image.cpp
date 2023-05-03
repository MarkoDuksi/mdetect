#include "image.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "CImg.h"


// public:
Image::Image(uint8_t* const img_buff, const uint16_t width, const uint16_t height) noexcept :
    m_data(img_buff),
    m_width(width),
    m_height(height),
    m_size(static_cast<uint32_t>(width) * height)
    {}

// public:
uint16_t Image::width() const noexcept {
    return m_width;
}

// public:
uint16_t Image::height() const noexcept {
    return m_height;
}

// public:
uint32_t Image::size() const noexcept {
    return m_size;
}


// public:
Image& Image::operator=(const Image& other) {
    if (this != &other) {
        assert(m_size == other.m_size &&
               "Image::operator= cannot assign between images of different sizes");

        m_width = other.m_width;
        m_height = other.m_height;
        std::memcpy(m_data, other.m_data, m_size);
    }

    return *this;
} 

// public:
uint8_t& Image::at(const uint16_t row, const uint16_t col) const {
    return m_data[row * m_width + col];
}

// public:
uint8_t Image::at(const int32_t row, const int32_t col, uint8_t pad_value) const noexcept {
    return row >= 0 && col >= 0 && row < m_height && col < m_width ?
        m_data[row * m_width + col] : pad_value;
}

// public:
Image& Image::threshold(const uint8_t threshold) noexcept {
    for (uint32_t idx = 0; idx < m_size; ++idx) {
        m_data[idx] = m_data[idx] <= threshold ? 0 : 255;
    }

    return *this;
}

// public:
Image& Image::absdiff(const Image& other) {
    assert (m_width == other.m_width && m_height == other.m_height &&
            "Image::absdiff cannot process images of different dimensions");

    for (uint32_t idx = 0; idx < m_size; ++idx) {
        m_data[idx] = std::abs(m_data[idx] - other.m_data[idx]);
    }

    return *this;
}

// public
void Image::save(const char* filename) const {
    namespace cimg = cimg_library;
    cimg::CImg<uint8_t> img_grey;
    img_grey.assign(m_data, m_width, m_height);
    img_grey.rotate(-90).save(filename);
}
