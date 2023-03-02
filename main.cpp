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

#define WIDTH_256 256
#define HEIGHT_192 192
#define SIZE_256x192 49152

#define WIDTH_254 254
#define HEIGHT_190 190
#define SIZE_254x190 48260

#define WIDTH_252 252
#define HEIGHT_188 188
#define SIZE_252x188 47376

#define WIDTH_250 250
#define HEIGHT_186 186
#define SIZE_250x186 46500

#define WIDTH_248 248
#define HEIGHT_184 184
#define SIZE_248x184 45632

#define WIDTH_246 246
#define HEIGHT_182 182
#define SIZE_246x182 44772

#define WIDTH_244 244
#define HEIGHT_180 180
#define SIZE_244x180 43920

#define WIDTH_232 232
#define HEIGHT_168 168
#define SIZE_232x168 38976

#define DEPTH_1 1
#define CHANNELS_3 3
#define CHANNELS_1 1
#define THRESHOLD_127 127


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

    const size_t start_offset = -(kernel.anchor_Y * img_width + kernel.anchor_X);
    const size_t next_row_offset = img_width - kernel.width;

    idx += start_offset;
    for (size_t i = 0; i < kernel.height; ++i) {
        for (size_t j = 0; j < kernel.width; ++j) {
            accumulator += kernel.elements[kernel_idx++] * image[idx++];
        }
        idx += next_row_offset;
    }
    return accumulator;
}

void convolve(const uint8_t* src_image, uint8_t* dst_image, size_t src_width, size_t src_height, const Kernel& kernel, const size_t stride, float (*normalizer)(float)) {
    size_t src_idx = kernel.anchor_Y * src_width + kernel.anchor_X;
    size_t dst_idx = 0;

    const size_t next_row_offset = (stride - 1) * src_width + kernel.width - stride;
    const size_t width = src_width - (kernel.width - 1);
    const size_t height = src_height - (kernel.height - 1);

    for (size_t i = 0; i < height; i += stride) {
        for (size_t j = 0; j < width; j += stride) {
            dst_image[dst_idx++] = normalizer(stamp(src_image, src_width, src_idx, kernel));
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
    const Kernel flat_kernel(kernel_elements, 4, 4, 0, 0);
    const size_t stride = 4;
    auto normalizer = [] (float x) { return x / 16.0f; };

    convolve(src_image, dst_image, src_width, src_height, flat_kernel, stride, normalizer);
}

void absdiff(const uint8_t* image1, const uint8_t* image2, uint8_t* abs_diff, const size_t img_size) {
    for (size_t i = 0; i < img_size; ++i) {
        abs_diff[i] = abs(image1[i] - image2[i]);
    }
}

void update_background(uint8_t* img_foreground, uint8_t* img_background, size_t img_size) {
    memcpy(img_background, img_foreground, img_size);
}

void threshold(const uint8_t* src_image, uint8_t* dst_image, const size_t img_size, const uint8_t threshold) {
    for (size_t i = 0; i < img_size; ++i) {
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
