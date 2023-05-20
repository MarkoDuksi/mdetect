#pragma once

#include <stdint.h>
#include <cassert>

#include "image.h"


namespace transformations {

template<typename T>
struct Kernel {
    const T* const elements;
    const uint8_t width;
    const uint8_t height;
    const uint8_t anchor_X;
    const uint8_t anchor_Y;

    Kernel(const T* const elements, const uint8_t width, const uint8_t height, const uint8_t anchor_X, const uint8_t anchor_Y) noexcept :
        elements(elements),
        width(width),
        height(height),
        anchor_X(anchor_X),
        anchor_Y(anchor_Y)
        {
            assert(anchor_X < width && anchor_Y < height &&
                   "Kernel anchor cannot be positioned outside of the kernel");
        }
};

template<typename T>
T stamp(const Image& image, const int32_t img_row, const int32_t img_col, const Kernel<T>& kernel) {
    T accumulator = 0;
    uint16_t kernel_idx = static_cast<uint16_t>(kernel.width) * kernel.height - 1;

    int32_t row = img_row - kernel.anchor_Y;
    for (uint8_t window_Y = 0; window_Y < kernel.height; ++window_Y, ++row) {
        int32_t col = img_col - kernel.anchor_X;
        for (uint8_t window_X = 0; window_X < kernel.width; ++window_X, ++col) {
            accumulator += (kernel.elements[kernel_idx--] * image.at(row, col, 0));
        }
    }

    return accumulator;
}

template<typename T>
void convolve(const Image& src_image, const Kernel<T>& kernel, const uint8_t stride_X, const uint8_t stride_Y, uint8_t (*postprocess)(T), Image& dst_image) {
    for (uint16_t dst_row = 0, src_row = 0; dst_row < dst_image.height(); ++dst_row, src_row += stride_Y) {
        for (uint16_t dst_col = 0, src_col = 0; dst_col < dst_image.width(); ++dst_col, src_col += stride_X) {
            dst_image.at(dst_row, dst_col) = postprocess(stamp<T>(src_image, src_row, src_col, kernel));
        }
    }
}

void threshold(Image& image, const uint8_t threshold) noexcept;
void absdiff(Image& image, const Image& other);
void downscale_4x4(const Image& src_image, Image& dst_image);
void downscale_8x8(const Image& src_image, Image& dst_image);
void dilate_8x8(const Image& src_image, Image& dst_image);
void dilate_13x13(const Image& src_image, Image& dst_image);
void erode_13x13(const Image& src_image, Image& dst_image);

}   // namespace transformations
