#include <iostream>
#include <stdint.h>
#include <sys/types.h>

#include "mdjpeg.h"

#include "tests/test-utils.h"
#include "MotionDetector.h"
#include "BoundingBox.h"
#include "Image.h"


int main(int argc, char** argv) {

    if (argc != 3) {

        std::cout << "usage: " << argv[0] << " input_directory output_directory\n";

        exit(1);
    }

    std::filesystem::path input_dir = argv[1];

    if (!std::filesystem::is_directory(input_dir)) {

        std::cerr << "invalid input directory: " << argv[1] << "\n";

        exit(1);
    }

    const auto input_paths = mdetect_test_utils::get_input_img_paths(input_dir);

    if (input_paths.size() == 0) {

        exit(0);
    }

    std::filesystem::path output_dir = argv[2];
    std::filesystem::create_directories(output_dir);

    constexpr uint16_t width = 1024;
    constexpr uint16_t height = 768;
    constexpr uint16_t downscaled_width = width / 8;
    constexpr uint16_t downscaled_height = height / 8;
    constexpr uint16_t dest_square_width = 64;
    constexpr uint16_t dest_square_height = 64;

    StaticImage<downscaled_width, downscaled_height> downscaled_img;

    const auto [buf, size] = mdetect_test_utils::read_raw_jpeg_from_file(input_paths[0]);

    JpegDecoder decoder;

    decoder.assign(buf, size);
    decoder.dc_luma_decode(downscaled_img.data(), 0, 0, downscaled_width, downscaled_height);

    MotionDetector<downscaled_width, downscaled_height> motion(downscaled_img);
    DownscalingBlockWriter<dest_square_width, dest_square_height> writer;
    StaticImage<dest_square_width, dest_square_height> dest_square;
    BoundingBox outer_bounds(0, 0, downscaled_width, downscaled_height);

    for (const auto& input_path : input_paths) {

        std::cout << "processing image: " << input_path << "\n";

        const auto [buf, size] = mdetect_test_utils::read_raw_jpeg_from_file(input_path);
        decoder.assign(buf, size);
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
