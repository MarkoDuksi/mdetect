#include "image.h"

#include <cstdlib>
#include <cassert>


// public:
Image::Image(uint8_t* const img_buff, const uint16_t width, const uint16_t height) noexcept :
    m_data(img_buff),
    m_data_len(static_cast<uint32_t>(width) * height),
    width(width),
    height(height)
    {};

// public:
uint8_t& Image::operator[](uint32_t idx) const {
    return m_data[idx];
}

// public:
uint8_t& Image::at(const uint16_t row, const uint16_t col) const noexcept {
    assert(row < height && col < width && "Image::at cannot access element outside of the image (consider using overload with padding)");

    return m_data[row * width + col];
}

// public:
uint8_t Image::at(const int32_t row, const int32_t col, uint8_t pad_value) const noexcept {
    return row < 0 || col < 0 || row >= height || col >= width ?
        pad_value : m_data[row * width + col];
}

// public:
Image& Image::threshold(const uint8_t threshold) noexcept {
    for (uint32_t idx = 0; idx < m_data_len; ++idx) {
        m_data[idx] = m_data[idx] < threshold ? 0 : 255;
    }

    return *this;
}

// public:
Image& Image::absdiff(const Image& other) {
    assert (width == other.width && height && other.height && "Image::absdiff cannot process images of different dimensions");

    for (uint32_t idx = 0; idx < m_data_len; ++idx) {
        m_data[idx] = std::abs(m_data[idx] - other.m_data[idx]);
    }

    return *this;
}
