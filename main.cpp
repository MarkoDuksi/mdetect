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

#define DEPTH_1 1
#define CHANNELS_3 3
#define CHANNELS_1 1

static uint8_t memory_pool[3 * SIZE_1024x768] = { 0 };
static uint8_t* background = memory_pool;
static uint8_t* img_curr = &memory_pool[SIZE_1024x768];
static uint8_t* img_diff = &memory_pool[2 * SIZE_1024x768];

void update_diff() {
    for (size_t i = 0; i < SIZE_1024x768; ++i) {
        img_diff[i] = std::abs(img_curr[i] - background[i]);
    }
}

int main() {
    std::vector<std::string> dir_content;
    for (const auto& entry : std::filesystem::directory_iterator("./captures")) {
        dir_content.push_back(entry.path().string());
    }
    std::sort(dir_content.begin(), dir_content.end());

    cimg::CImg<uint8_t> img_rgb(WIDTH_1024, HEIGHT_768, DEPTH_1, CHANNELS_3);
    cimg::CImg<uint8_t> img_greyscale(WIDTH_1024, HEIGHT_768, DEPTH_1, CHANNELS_1);
    cimg::CImgDisplay disp;

    for (const auto& filename : dir_content) {
        img_rgb.assign(filename.c_str());
        memcpy(img_curr, img_rgb._data + SIZE_1024x768, SIZE_1024x768); // copy only the green channel
        update_diff();
        img_greyscale.assign(img_diff, WIDTH_1024, HEIGHT_768);
        disp = img_greyscale;
        std::swap(background, img_curr);
        // disp.wait(100);
    }

    return 0;
}

// int main() {
//     std::vector<std::string> dir_content;
//     for (const auto& entry : std::filesystem::directory_iterator("./captures")) {
//         dir_content.push_back(entry.path().string());
//     }
//     std::sort(dir_content.begin(), dir_content.end());

//     cimg::CImgDisplay disp;

//     cimg::CImg<uint8_t> img_rgb(dir_content[0].c_str());

//     size_t width = img_rgb.width();
//     size_t height = img_rgb.height();
//     size_t size = width * height;

//     cimg::CImg<uint8_t> img_greyscale(width, height);

//     uint8_t* img0 = new uint8_t[size];
//     uint8_t* img1 = new uint8_t[size];
//     uint8_t* img_diff = new uint8_t[size];
//     uint8_t* imgs[] = {img0, img1};

//     size_t prev = 0;
//     size_t curr = 1;

//     memcpy(imgs[prev], img_rgb._data + size, size);

//     for (auto path_string_it = dir_content.begin() + 1; path_string_it != dir_content.end(); ++path_string_it) {
//         img_rgb.assign(path_string_it->c_str());
//         memcpy(imgs[curr], img_rgb._data + size, size);
//         for (size_t i = 0; i < size; ++i) {
//             img_diff[i] = std::abs(imgs[curr][i] - imgs[prev][i]);
//         }
//         img_greyscale.assign(img_diff, width, height);
//         disp = img_greyscale;
        
//         std::swap(prev, curr);
//         disp.wait(100);
//     }

//     delete img0;
//     delete img1;
//     delete img_diff;

//     return 0;
// }
