#include <stdint.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "CImg.h"
#include "MotionDetector.h"
#include "BoundingBox.h"
#include "Image.h"
#include "transform.h"

#define WIDTH_1024 static_cast<uint16_t>(1024)
#define HEIGHT_768 static_cast<uint16_t>(768)
#define WIDTH_128 static_cast<uint16_t>(128)
#define HEIGHT_96 static_cast<uint16_t>(96)


namespace cimg = cimg_library;
static const uint8_t green[] {0, 255, 0};


std::vector<std::filesystem::path> getInputImgPaths(const char* input_dir) {

    std::vector<std::filesystem::path> input_paths;

    for (const auto& dir_entry : std::filesystem::directory_iterator(input_dir)){

        if (dir_entry.path().extension() == ".jpg") {

            input_paths.push_back(dir_entry.path());
        }
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

    auto input_paths = getInputImgPaths(input_dir);

    cimg::CImg<uint8_t> img_rgb;

    img_rgb.assign(input_paths[0].c_str());

    Image full_size_ref_img(WIDTH_1024, HEIGHT_768, &img_rgb._data[WIDTH_1024 * HEIGHT_768]);

    StaticImage<WIDTH_128, HEIGHT_96> downscaled_ref_img;
    StaticImage<WIDTH_128, HEIGHT_96> downscaled_img;

    transform::downscale_8x8(full_size_ref_img, downscaled_ref_img);
    MotionDetector<WIDTH_128, HEIGHT_96> motion(downscaled_ref_img);

    for (const auto& input_path : input_paths) {

        std::cout << "processing image: " << input_path << std::endl;

        img_rgb.assign(input_path.c_str());

        Image full_size_img(WIDTH_1024, HEIGHT_768, &img_rgb._data[WIDTH_1024 * HEIGHT_768]);
        transform::downscale_8x8(full_size_img, downscaled_img);
        
        motion.detect(downscaled_img);

        bool motion_detected = false;
        while (auto bbox = motion.get_bbox()) {
            motion_detected = true;
            img_rgb.draw_rectangle(8 * bbox->topleft_X(), 8 * bbox->topleft_Y(), 8 * (bbox->bottomright_X()) - 1, 8 * (bbox->bottomright_Y()) - 1, green, 1, ~0U);
        }

        if (motion_detected) {
            img_rgb.rotate(-90).save((std::string(output_dir) + "/" + input_path.filename().c_str()).c_str());
        }

        motion.set_reference(downscaled_img);
    }

    return 0;
}
