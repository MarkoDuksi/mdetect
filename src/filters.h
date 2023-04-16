#pragma once

#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <limits>


namespace filters {

    template<typename T>
    struct Kernel {
        const T* elements;
        const uint8_t width, height;
        const uint8_t anchor_X, anchor_Y;

        Kernel(const T* elements, const uint8_t width, const uint8_t height, const uint8_t anchor_X = 0, const uint8_t anchor_Y = 0) :
            elements(elements),
            width(width),
            height(height),
            anchor_X(anchor_X),
            anchor_Y(anchor_Y)
            {}
    };

    // one-time apply rotated kernel over the neighbourhood of image[idx], no bounds checking
    template<typename T, typename K>
    K stamp(const T* image, const uint16_t img_width, uint32_t idx, const Kernel<K>& kernel) {
        K accumulator = 0;
        uint16_t kernel_idx = kernel.width * kernel.height - 1;

        const uint32_t start_offset = -(kernel.anchor_Y * img_width + kernel.anchor_X);
        const uint16_t next_row_offset = img_width - kernel.width;

        idx += start_offset;
        for (size_t row = 0; row < kernel.height; ++row) {
            for (size_t col = 0; col < kernel.width; ++col) {
                accumulator += kernel.elements[kernel_idx--] * image[idx++];
            }
            idx += next_row_offset;
        }

        return accumulator;
    }

    template<typename T, typename K>
    void convolve(const T* src_image, const uint16_t src_img_width, const uint16_t src_img_height, const Kernel<K>& kernel, const uint8_t stride, T (*cb_postprocess)(K), T* dst_image) {
        uint32_t src_idx = kernel.anchor_Y * src_img_width + kernel.anchor_X;
        uint32_t dst_idx = 0;

        const uint32_t next_row_offset = (stride - 1) * src_img_width + kernel.width - stride;
        const uint16_t width = src_img_width - (kernel.width - 1);
        const uint16_t height = src_img_height - (kernel.height - 1);

        for (size_t row = 0; row < height; row += stride) {
            for (size_t col = 0; col < width; col += stride) {
                dst_image[dst_idx++] = cb_postprocess(stamp(src_image, src_img_width, src_idx, kernel));
                src_idx += stride;
            }
            src_idx += next_row_offset;
        }
    }

    template<typename T>
    void downscale(const T* src_image, const uint16_t src_img_width, const uint16_t src_img_height, T* dst_image) {
        const float kernel_elements[] {
            1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F
        };
        const Kernel flat_kernel(kernel_elements, 4, 4, 0, 0);
        const uint8_t stride = 4;
        auto postprocess = [](float x) { return (T)(x / 16.0F); };

        convolve<T, float>(src_image, src_img_width, src_img_height, flat_kernel, stride, postprocess, dst_image);
    }

    template<typename T>
    void absdiff(const T* src_image1, const T* src_image2, const uint16_t src_img_width, const uint16_t src_img_height, T* dst_image) {
        const uint32_t img_size = src_img_width * src_img_height;
        for (size_t idx = 0; idx < img_size; ++idx) {
            dst_image[idx] = std::abs(src_image1[idx] - src_image2[idx]);
        }
    }

    template<typename T>
    void threshold(const T* src_image, const uint16_t src_img_width, const uint16_t src_img_height, const T threshold, T* dst_image) {
        const uint32_t img_size = src_img_width * src_img_height;
        for (size_t idx = 0; idx < img_size; ++idx) {
            dst_image[idx] = src_image[idx] < threshold ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
        }
    }

    template<typename T>
    void pad(const T* src_image, const uint16_t src_img_width, const uint16_t src_img_height, const T pad_value, const uint8_t pad_size, T* dst_image) {
        const uint32_t padding_block_size = pad_size * (src_img_width + 2 * pad_size);
        
        // fill the top margin
        std::memset(dst_image, pad_value, padding_block_size * sizeof(T));
        dst_image += padding_block_size;

        for (size_t src_row = 0; src_row < src_img_height; ++src_row) {

            // fill the left margin for the current row only
            std::memset(dst_image, pad_value, pad_size * sizeof(T));
            dst_image += pad_size;

            // append the current src_img row data
            std::memcpy(dst_image, src_image, src_img_width * sizeof(T));
            src_image += src_img_width;
            dst_image += src_img_width;

            // fill the right margin for the current row only
            std::memset(dst_image, pad_value, pad_size * sizeof(T));
            dst_image += pad_size;
        }

        // fill the bottom margin
        std::memset(dst_image, pad_value, padding_block_size * sizeof(T));
    }

    template<typename T>
    void dilate(const T* src_image, const uint16_t src_img_width, const uint16_t src_img_height, T* dst_image) {
        const float kernel_elements[] {
            0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F
        };
        const Kernel round_kernel(kernel_elements, 13, 13, 6, 6);
        const uint8_t stride = 1;
        auto postprocess = [](float x) { return !x ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max(); };

        convolve<T, float>(src_image, src_img_width, src_img_height, round_kernel, stride, postprocess, dst_image);
    }

    template<typename T>
    void erode(const T* src_image, const uint16_t src_img_width, const uint16_t src_img_height, T* dst_image) {
        const float kernel_elements[] {
            0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
            1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F,
            0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F,
            0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 1.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F
        };
        const Kernel round_kernel(kernel_elements, 13, 13, 6, 6);
        const uint8_t stride = 1;
        auto postprocess = [](float x) { return x < 120 * std::numeric_limits<T>::max() ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max(); };

        convolve<T, float>(src_image, src_img_width, src_img_height, round_kernel, stride, postprocess, dst_image);
    }

}