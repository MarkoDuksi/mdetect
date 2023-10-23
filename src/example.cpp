#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <string>
#include <stdint.h>

#include "mdjpeg.h"

#include "MotionDetector.h"
#include "BoundingBox.h"
#include "Image.h"


std::vector<std::filesystem::path> get_input_img_paths(const std::filesystem::path& input_dir);
std::pair<uint8_t*, size_t> read_raw_jpeg_from_file(const std::filesystem::path& file_path);

int main(int argc, char** argv) {

    if (argc != 3) {

        std::cout << "usage: " << argv[0] << " input_directory output_directory\n";

        exit(1);
    }

    std::filesystem::path input_dir = argv[1];
    std::filesystem::path output_dir = argv[2];

    constexpr uint16_t width = 1024;
    constexpr uint16_t height = 768;
    constexpr uint16_t downscaled_width = width / 8;
    constexpr uint16_t downscaled_height = height / 8;
    constexpr uint16_t dest_square_width = 64;
    constexpr uint16_t dest_square_height = 64;

    StaticImage<downscaled_width, downscaled_height> downscaled_img;

    const auto input_paths = get_input_img_paths(input_dir);
    const auto [buf, size] = read_raw_jpeg_from_file(input_paths[0]);

    {
        JpegDecoder ref_decoder(buf, size);
        ref_decoder.dc_luma_decode(downscaled_img.data(), 0, 0, downscaled_width, downscaled_height);
    }

    MotionDetector<downscaled_width, downscaled_height> motion(downscaled_img);
    DownscalingBlockWriter<dest_square_width, dest_square_height> writer;
    StaticImage<dest_square_width, dest_square_height> dest_square;
    BoundingBox outer_bounds(0, 0, downscaled_width, downscaled_height);

    for (const auto& input_path : input_paths) {

        std::cout << "processing image: " << input_path << "\n";

        const auto [buf, size] = read_raw_jpeg_from_file(input_path);
        JpegDecoder decoder(buf, size);
        decoder.dc_luma_decode(downscaled_img.data(), 0, 0, downscaled_width, downscaled_height);

        motion.detect(downscaled_img);

        uint counter = 0;
        while (auto bbox = motion.get_bounding_box()) {

            if (std::min(bbox->width(), bbox->height()) >= 8) {

                if (!bbox->expand_to_square(outer_bounds)) {

                    continue;
                }

                decoder.luma_decode(dest_square.data(), bbox->topleft_X, bbox->topleft_Y, bbox->bottomright_X, bbox->bottomright_Y, writer);

                std::filesystem::path output_path = output_dir / input_path.stem();
                output_path += "_" + std::to_string(counter++) + ".pgm";
                dest_square.save_as_pgm(output_path);
            }
        }

        motion.set_reference(downscaled_img);
    }

    return 0;
}

std::vector<std::filesystem::path> get_input_img_paths(const std::filesystem::path& input_dir) {

    std::vector<std::filesystem::path> input_paths;

    for (const auto& dir_entry : std::filesystem::directory_iterator(input_dir)){

        if (dir_entry.path().extension() == ".jpg") {

            input_paths.push_back(dir_entry.path());
        }
    }

    std::sort(input_paths.begin(), input_paths.end());

    return input_paths;
}

std::pair<uint8_t*, size_t> read_raw_jpeg_from_file(const std::filesystem::path& file_path) {

    std::ifstream file(file_path, std::ios::in | std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {

        std::cout << "Error opening input file: " << file_path.c_str() << "\n";

        return {nullptr, 0};
    }

    const auto size = file.tellg();
    char* raw_jpeg = new char[size];

    file.seekg(0);
    file.read(raw_jpeg, size);

    if (file.gcount() != size) {

        std::cout << "Error reading input file: " << file_path.c_str() << "\n";
        file.close();

        return {nullptr, 0};
    }

    file.close();

    return {reinterpret_cast<uint8_t*>(raw_jpeg), size};
}
