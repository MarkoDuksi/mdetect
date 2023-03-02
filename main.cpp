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

void dilate(const uint8_t* src_image, uint8_t* dst_image, const size_t src_width, const size_t src_height) {
    const float kernel_elements[] = {
        0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0
    };
    const Kernel round_kernel(kernel_elements, 13, 13, 6, 6);
    const size_t stride = 1;
    auto normalizer = [](float res) { return !res ? 0.0f : 255.0f; };

    convolve(src_image, dst_image, src_width, src_height, round_kernel, stride, normalizer);
}

void erode(const uint8_t* src_image, uint8_t* dst_image, const size_t src_width, const size_t src_height) {
    const float kernel_elements[] = {
        0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0
    };
    const Kernel round_kernel(kernel_elements, 13, 13, 6, 6);
    const size_t stride = 1;
    auto normalizer = [](float res) { return res < 30600.0f ? 0.0f : 255.0f; };

    convolve(src_image, dst_image, src_width, src_height, round_kernel, stride, normalizer);
}


int main() {
    uint8_t* img_input;
    uint8_t img_downscaled[SIZE_256x192];
    uint8_t img_background[SIZE_256x192] = { 0 };
    uint8_t img_diff[SIZE_256x192];
    uint8_t img_binarized[SIZE_256x192];
    // uint8_t img_dilated2[SIZE_254x190];
    // uint8_t img_dilated4[SIZE_252x188];
    // uint8_t img_dilated6[SIZE_250x186];
    // uint8_t img_dilated8[SIZE_248x184];
    // uint8_t img_dilated10[SIZE_246x182];
    uint8_t img_dilated[SIZE_244x180];
    uint8_t img_eroded[SIZE_232x168];

    // filenames from 'captures/' into sorted std::vector<std::string>
    std::vector<std::string> dir_content;
    for (const auto& entry : std::filesystem::directory_iterator("./captures")) {
        dir_content.push_back(entry.path().string());
    }
    std::sort(dir_content.begin(), dir_content.end());

    cimg::CImg<uint8_t> img_rgb;
    cimg::CImg<uint8_t> img1;
    // cimg::CImg<uint8_t> img2;
    // cimg::CImg<uint8_t> img3;
    cimg::CImg<uint8_t> img4;
    cimg::CImgDisplay disp1;
    // cimg::CImgDisplay disp2;
    // cimg::CImgDisplay disp3;
    cimg::CImgDisplay disp4;

    for (const auto& filename : dir_content) {
        img_rgb.assign(filename.c_str());

        // point to the green channel
        img_input = &img_rgb._data[SIZE_1024x768];

        downscale(img_input, img_downscaled, WIDTH_1024, HEIGHT_768);
        absdiff(img_downscaled, img_background, img_diff, SIZE_256x192);
        threshold(img_diff, img_binarized, SIZE_256x192, THRESHOLD_127);
        // dilate2(img_binarized, img_dilated2, WIDTH_256, HEIGHT_192);
        // dilate4(img_binarized, img_dilated4, WIDTH_256, HEIGHT_192);
        // dilate6(img_binarized, img_dilated6, WIDTH_256, HEIGHT_192);
        // dilate8(img_binarized, img_dilated8, WIDTH_256, HEIGHT_192);
        dilate(img_binarized, img_dilated, WIDTH_256, HEIGHT_192);
        erode(img_dilated, img_eroded, WIDTH_244, HEIGHT_180);

        update_background(img_downscaled, img_background, SIZE_256x192);

        img1.assign(img_downscaled, WIDTH_256, HEIGHT_192);
        // img2.assign(img_diff, WIDTH_256, HEIGHT_192);
        // img3.assign(img_binarized, WIDTH_256, HEIGHT_192);
        // img4.assign(img_dilated2, WIDTH_254, HEIGHT_190);
        // img4.assign(img_dilated6, WIDTH_250, HEIGHT_186);
        // img4.assign(img_dilated8, WIDTH_248, HEIGHT_184);
        img4.assign(img_eroded, WIDTH_232, HEIGHT_168);

        disp1 = img1;
        // disp2 = img2;
        // disp3 = img3;
        disp4 = img4;

        // disp.wait(100);
    }

    return 0;
}
