#include <stdint.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "CImg.h"
#include "bbox.h"
#include "motion.h"

#define WIDTH_1024 1024
#define HEIGHT_768 768
#define MIN_BBOX_DIM 30


namespace cimg = cimg_library;
static const uint8_t green[] {0, 255, 0};


std::vector<std::filesystem::path> getInputImgPaths(const char* input_dir) {
    std::vector<std::filesystem::path> input_paths;

    for (const auto& dir_entry : std::filesystem::directory_iterator(input_dir)){
        if (dir_entry.path().extension() == ".jpg")
            input_paths.push_back(dir_entry.path());
    }
    std::sort(input_paths.begin(), input_paths.end());

    return input_paths;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "usage: " << argv[0] << " input_directory output_directory" << std::endl;
        exit(1);
    }

    char* input_dir = argv[1];
    char* output_dir = argv[2];

    cimg::CImg<uint8_t> img_rgb1;
    cimg::CImg<uint8_t> img_rgb2;

    uint8_t* full_size_img;

    auto input_paths = getInputImgPaths(input_dir);

    img_rgb1.assign(input_paths[0].c_str());
    full_size_img = &img_rgb1._data[WIDTH_1024 * HEIGHT_768];

    MotionDetector motion(WIDTH_1024, HEIGHT_768, MIN_BBOX_DIM, full_size_img, full_size_img);

    for (const auto& input_path : input_paths) {
        std::cout << "processing image: " << input_path << std::endl;

        img_rgb2.assign(input_path.c_str());
        full_size_img = &img_rgb2._data[WIDTH_1024 * HEIGHT_768];
        auto bboxes = motion.detect(full_size_img);

        if (bboxes.size()) {
            // uint subframe_count = 1;
            for (auto bbox : bboxes) {
                img_rgb2.draw_rectangle(4 * bbox.topleft_X, 4 * bbox.topleft_Y, 4 * (bbox.bottomright_X) - 1, 4 * (bbox.bottomright_Y) - 1, green, 1, ~0U);
                // img_rgb2.get_crop(4 * bbox.topleft_X, 4 * bbox.topleft_Y, 4 * (bbox.bottomright_X) - 1, 4 * (bbox.bottomright_Y) - 1)
                //        .rotate(-90)
                //        .save((std::string(output_dir) + "/" + input_path.stem().c_str() + "_" + std::to_string(subframe_count++) + ".jpg").c_str());
            }
            img_rgb2.rotate(-90).save((std::string(output_dir) + "/" + input_path.filename().c_str()).c_str());
        }
    }

    return 0;
}