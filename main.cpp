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

void update_diff(uint8_t* image1, uint8_t* image2, uint8_t* diff, size_t width, size_t height) {
    size_t size = width * height;
    for (size_t i = 0; i < size; ++i) {
        diff[i] = std::abs(image1[i] - image2[i]);
    }
}

void update_background(uint8_t*& background, uint8_t*& img_curr) {
    std::swap(background, img_curr);
}

void inplace_downscale(uint8_t* image, size_t width, size_t height, size_t factor) {
    size_t src_idx = 0;
    size_t dst_idx = 0;
    size_t box_idx = 0;
    uint16_t accumulator;

    for (size_t src_row = 0; src_row < height; src_row += factor) {
        for (size_t src_col = 0; src_col < width; src_col += factor) {
            src_idx = src_row * width + src_col;
            accumulator = 0;
            for (size_t box_row = 0; box_row < factor; ++box_row) {
                for (size_t box_col = 0; box_col < factor; ++box_col) {
                    box_idx = src_idx + box_row * width + box_col;
                    accumulator += image[box_idx];
                }
            }
            image[dst_idx++] = accumulator / (factor * factor);
        }
    }
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

        inplace_downscale(img_curr, WIDTH_1024, HEIGHT_768, 4);
        update_diff(background, img_curr, img_diff, WIDTH_256, HEIGHT_192);
        update_background(background, img_curr);

        img_greyscale.assign(img_diff, WIDTH_256, HEIGHT_192);
        disp = img_greyscale;
        // disp.wait(100);
    }

    return 0;
}
