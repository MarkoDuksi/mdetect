#pragma once

#include <cstdlib>


namespace filters {

template<typename T>
struct Kernel {
    const T* elements;
    const size_t width, height;
    const size_t anchor_X, anchor_Y;

    Kernel(const T* elements, const size_t width, const size_t height, 
           const size_t anchor_X = 0, const size_t anchor_Y = 0) :
        elements(elements),
        width(width),
        height(height),
        anchor_X(anchor_X),
        anchor_Y(anchor_Y)
        {}
};

// helper func: one-time apply rotated kernel over the neighbourhood of image[idx] (no bounds checking)
template<typename T, typename K>
K stamp(const T* image, const size_t img_width, size_t idx, const Kernel<K>& kernel){
    size_t kernel_idx = kernel.width * kernel.height - 1;
    K accumulator = 0;

    const size_t start_offset = -(kernel.anchor_Y * img_width + kernel.anchor_X);
    const size_t next_row_offset = img_width - kernel.width;

    idx += start_offset;
    for (size_t i = 0; i < kernel.height; ++i) {
        for (size_t j = 0; j < kernel.width; ++j) {
            accumulator += kernel.elements[kernel_idx--] * image[idx++];
        }
        idx += next_row_offset;
    }

    return accumulator;
}

template<typename T, typename K>
void convolve(const T* src_image, size_t src_width, size_t src_height, const Kernel<K>& kernel, const size_t stride, T (*cb_normalize)(K), T* dst_image) {
    size_t src_idx = kernel.anchor_Y * src_width + kernel.anchor_X;
    size_t dst_idx = 0;

    const size_t next_row_offset = (stride - 1) * src_width + kernel.width - stride;
    const size_t width = src_width - (kernel.width - 1);
    const size_t height = src_height - (kernel.height - 1);

    for (size_t i = 0; i < height; i += stride) {
        for (size_t j = 0; j < width; j += stride) {
            dst_image[dst_idx++] = cb_normalize(stamp(src_image, src_width, src_idx, kernel));
            src_idx += stride;
        }
        src_idx += next_row_offset;
    }
}

template<typename T>
void downscale(const T* src_image, const size_t src_width, const size_t src_height, T* dst_image) {
    const float kernel_elements[] = {
        1.0F, 1.0F, 1.0F, 1.0F,
        1.0F, 1.0F, 1.0F, 1.0F,
        1.0F, 1.0F, 1.0F, 1.0F,
        1.0F, 1.0F, 1.0F, 1.0F
    };
    const Kernel flat_kernel(kernel_elements, 4, 4, 0, 0);
    const size_t stride = 4;
    auto normalize = [] (float x) { return (uint8_t) (x / 16.0F); };

    convolve<T, float>(src_image, src_width, src_height, flat_kernel, stride, normalize, dst_image);
}

template<typename T>
void absdiff(const T* image1, const T* image2, const size_t src_width, const size_t src_height, T* abs_diff) {
    const size_t img_size = src_width * src_height;
    for (size_t i = 0; i < img_size; ++i) {
        abs_diff[i] = std::abs(image1[i] - image2[i]);
    }
}

template<typename T>
void threshold(const T* src_image, const size_t src_width, const size_t src_height, const T threshold, T* dst_image) {
    const size_t img_size = src_width * src_height;
    for (size_t i = 0; i < img_size; ++i) {
        dst_image[i] = src_image[i] < threshold ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
    }
}

template<typename T>
void pad(T* src_image, const size_t src_width, const size_t src_height, const T pad_value, const size_t pad_size, T* dst_image) {
    const size_t twice_pad_size = 2 * pad_size;
    const size_t padding_top_size = pad_size * (src_width + twice_pad_size + 1);
    const size_t padding_bottom_size = pad_size * (src_width + twice_pad_size - 1);
    
    // top padding
    memset(dst_image, pad_value, padding_top_size * sizeof(T));
    dst_image += padding_top_size;

    // left padding + image + right padding
    for (size_t i = 0; i < src_height; ++i) {
        memcpy(dst_image, src_image, src_width * sizeof(T));
        dst_image += src_width;
        src_image += src_width;
        memset(dst_image, pad_value, twice_pad_size * sizeof(T));
        dst_image += twice_pad_size;
    }

    // bottom padding
    memset(dst_image, pad_value, padding_bottom_size * sizeof(T));
}

template<typename T>
void dilate(const T* src_image, const size_t src_width, const size_t src_height, T* dst_image) {
    const float kernel_elements[] = {
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
    const size_t stride = 1;
    auto normalize = [](float x) { return !x ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max(); };

    convolve<T, float>(src_image, src_width, src_height, round_kernel, stride, normalize, dst_image);
}

template<typename T>
void erode(const T* src_image, const size_t src_width, const size_t src_height, T* dst_image) {
    const float kernel_elements[] = {
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
    const size_t stride = 1;
    auto normalize = [](float x) { return x < 120 * std::numeric_limits<T>::max() ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max(); };

    convolve<T, float>(src_image, src_width, src_height, round_kernel, stride, normalize, dst_image);
}

}