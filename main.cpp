#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "CImg.h"

namespace cimg = cimg_library;

#define WIDTH_1024 1024
#define HEIGHT_768 768
#define SIZE_1024x768 786432

#define WIDTH_512 512
#define HEIGHT_384 384
#define SIZE_512x384 196608

#define WIDTH_256 256
#define HEIGHT_192 192
#define SIZE_256x192 49152

#define DEPTH_1 1
#define CHANNELS_3 3
#define CHANNELS_1 1
#define THRESHOLD_150 150


struct Kernel {
    const float* elements;
    const size_t width, height;
    const size_t anchor_X, anchor_Y;

    Kernel(const float* elements, const size_t width, const size_t height, 
           const size_t anchor_X = 0, const size_t anchor_Y = 0) :
        elements(elements),
        width(width),
        height(height),
        anchor_X(anchor_X),
        anchor_Y(anchor_Y)
        {}
};

// helper func: one-time apply kernel over the neighbourhood of image[idx] (no bounds checking)
float stamp(const uint8_t *image, const size_t img_width, size_t idx, const Kernel& kernel){
    size_t kernel_idx = 0;
    float accumulator = 0;

    const size_t start_offset = kernel.anchor_Y * img_width + kernel.anchor_X;
    const size_t next_row_offset = img_width - kernel.width;

    idx -= start_offset;
    for (size_t i = 0; i < kernel.height; ++i) {
        for (size_t j = 0; j < kernel.width; ++j) {
            accumulator += kernel.elements[kernel_idx++] * image[idx++];
        }
        idx += next_row_offset;
    }
    return accumulator;
}

void convolve(const uint8_t* src_image, uint8_t* dst_image, const size_t img_width, const size_t img_height, const Kernel& kernel, const size_t stride, float (*scaler)(float)) {
    size_t src_idx = img_width * kernel.anchor_Y + kernel.anchor_X;
    size_t dst_idx = 0;

    const size_t next_row_offset = (stride - 1) * img_width;

    for (size_t i = 0; i < img_height; i += stride) {
        for (size_t j = 0; j < img_width; j += stride) {
            dst_image[dst_idx++] = scaler(stamp(src_image, img_width, src_idx, kernel));
            src_idx += stride;
        }
        src_idx += next_row_offset;
    }
}

void downscale(const uint8_t* src_image, uint8_t* dst_image, const size_t src_width, const size_t src_height) {
    const float kernel_elements[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    const Kernel avg_kernel(kernel_elements, 4, 4, 0, 0);
    const size_t stride = 4;
    auto scaler = [] (float x) { return x / 16.0f; };

    convolve(src_image, dst_image, src_width, src_height, avg_kernel, stride, scaler);
}

void absdiff(const uint8_t* image1, const uint8_t* image2, uint8_t* abs_diff, const size_t img_width, const size_t img_height) {
    const size_t size = img_width * img_height;
    for (size_t i = 0; i < size; ++i) {
        abs_diff[i] = abs(image1[i] - image2[i]);
    }
}

void update_background(uint8_t*& background, uint8_t*& img_curr) {
    std::swap(background, img_curr);
}

void threshold(const uint8_t* src_image, uint8_t* dst_image, const size_t img_width, const size_t img_height, const uint8_t threshold) {
    const size_t size = img_width * img_height;

    for (size_t i = 0; i < size; ++i) {
        dst_image[i] = src_image[i] < threshold ? 0 : 255;
    }
}

void erode(const uint8_t* src_image, uint8_t* dst_image, const size_t img_width, const size_t img_height) {

}

void dilate(const uint8_t* src_image, uint8_t* dst_image, const size_t width, const size_t height) {

}


int main() {
    // multi-purpose stack memory
    uint8_t memory_pool[3 * SIZE_1024x768] = { 0 };
    uint8_t* background = memory_pool;
    uint8_t* img_curr = &memory_pool[SIZE_1024x768];
    uint8_t* img_diff = &memory_pool[2 * SIZE_1024x768];

    // filenames from 'captures/' into sorted std::vector<std::string>
    std::vector<std::string> dir_content;
    for (const auto& entry : std::filesystem::directory_iterator("./captures")) {
        dir_content.push_back(entry.path().string());
    }
    std::sort(dir_content.begin(), dir_content.end());

    // image containers and main diplay
    cimg::CImg<uint8_t> img_rgb;
    cimg::CImg<uint8_t> img_greyscale;
    cimg::CImgDisplay disp;

    for (const auto& filename : dir_content) {
        img_rgb.assign(filename.c_str());

        // copy only the green channel
        memcpy(img_curr, img_rgb._data + SIZE_1024x768, SIZE_1024x768);

        downscale(img_curr, img_curr, WIDTH_1024, HEIGHT_768);
        absdiff(background, img_curr, img_diff, WIDTH_256, HEIGHT_192);
        threshold(img_diff, img_diff, WIDTH_256, HEIGHT_192, THRESHOLD_150);
        // erode(img_diff, img_eroded, WIDTH_256, HEIGHT_192);
        update_background(background, img_curr);

        img_greyscale.assign(img_diff, WIDTH_256, HEIGHT_192);
        disp = img_greyscale;
        // disp.wait(100);
    }

    return 0;
}
