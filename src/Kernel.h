#pragma once

#include <stdint.h>
#include <cassert>

#include "Image.h"


template<typename T>
class Kernel {

    public:

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

        void convolve(const Image& src_image, const uint8_t stride_X, const uint8_t stride_Y, uint8_t (*postprocess)(T), Image& dst_image) const {

            for (uint16_t dst_row = 0, src_row = 0; dst_row < dst_image.height; ++dst_row, src_row += stride_Y) {

                for (uint16_t dst_col = 0, src_col = 0; dst_col < dst_image.width; ++dst_col, src_col += stride_X) {

                    dst_image.at(dst_row, dst_col) = postprocess(stamp(src_image, src_row, src_col));
                }
            }
        }

    private:

        T stamp(const Image& image, const int32_t img_row, const int32_t img_col) const {

            T accumulator = 0;
            uint16_t kernel_idx = static_cast<uint16_t>(width) * height - 1;

            int32_t row = img_row - anchor_Y;
            for (uint8_t window_Y = 0; window_Y < height; ++window_Y, ++row) {

                int32_t col = img_col - anchor_X;
                for (uint8_t window_X = 0; window_X < width; ++window_X, ++col) {

                    accumulator += (elements[kernel_idx--] * image.at(row, col, 0));
                }
            }

            return accumulator;
        }
};
