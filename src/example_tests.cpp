#include <stdint.h>
#include <sys/types.h>
#include <iostream>

#include "mdjpeg.h"

#include "JpegMotionDetector.h"


int main(int argc, char** argv) {

    if (argc != 3) {

        std::cout << "usage: " << argv[0] << " input_directory output_directory\n";

        return 1;
    }

    std::filesystem::path input_dir = argv[1];

    // for testing/example purposes input image data is obtained from files within `input_dir`
    if (!std::filesystem::is_directory(input_dir)) {

        std::cerr << "invalid input directory: " << input_dir << "\n";

        return 1;
    }

    // get only the paths for filenames matching "`input_dir`/*.jpg"
    const auto input_paths = mdjpeg_test_utils::get_input_img_paths(input_dir);

    // motion detection can only find "movements" between multiple frames
    if (input_paths.size() < 2) {

        std::cout << "nothing to do in: " << input_dir << "\n";

        return 0;
    }

    // for testing/example purposes output image data is stored to files in `output_dir`
    std::filesystem::path output_dir = argv[2];
    std::filesystem::create_directories(output_dir);


    // use input images resolution of 1024x768 pixels
    constexpr uint16_t width = 1024;
    constexpr uint16_t height = 768;

    // use 1/8 of the input full resolution to perform motion detection on
    constexpr uint16_t downscaled_width = width / 8;
    constexpr uint16_t downscaled_height = height / 8;

    // use output images resolution of 64x64 pixels
    // (these do not have to be multiples of 8)
    constexpr uint16_t dest_width = 64;
    constexpr uint16_t dest_height = 64;

    // create a small buffer for raw pixel data of individual detected "movements"
    uint8_t dest_buff[dest_width * dest_height];

    // `downscaling_block_writer` needed for writing oversized input to fixed-sized `dest_buff`
    DownscalingBlockWriter<dest_width, dest_height> downscaling_block_writer;

    // a single JPEG decoder object used by motion detector as well as `main`
    JpegDecoder jpeg_decoder;

    // create `motion_detector` that internally operates with frame resolution of
    // `downscaled_width` x `downscaled_height` pixels
    // (using 1:8 scale is recommended for noise reduction and maximum efficiency)
    JpegMotionDetector<downscaled_width, downscaled_height> motion_detector(jpeg_decoder);

    // `input_paths` is used to mock a steady stream of images coming from a camera
    auto input_path_it = input_paths.begin();


    // `input_paths` is purposely not indefinite; real stream would be
    while (input_path_it != input_paths.end()) {

        // read compressed data from an input image into `buffer` of size `size`
        // (for testing/example purposes the data buffer is allocated on the heap here)
        const auto [buffer, size] = mdjpeg_test_utils::read_raw_jpeg_from_file(*input_path_it++);
        
        // set initial reference frame
        const bool reference_success = motion_detector.set_reference(buffer, size);

        // clean up after use of image data (depends on the source of image data)
        delete[] buffer;

        if (reference_success) {

            break;
        }
    }
    

    // process the rest of input images
    // `input_paths` is purposely not indefinite; real stream would be
    while (input_path_it != input_paths.end()) {

        std::cout << "processing image: " << *input_path_it << "\n";

        // read compressed data from an input image into `buffer` of size `size`
        // (for testing/example purposes the data buffer is allocated on the heap here)
        const auto [buffer, size] = mdjpeg_test_utils::read_raw_jpeg_from_file(*input_path_it);

        // detect "movements" in the current frame with respect to the reference frame
        const uint8_t detection_threshold = 127;
        const int movements_count = motion_detector.detect(buffer, size, detection_threshold);

        if (movements_count < 0) {

            std::cout << "   JPEG decompression FAILED.\n";

            // clean up after use of image data (depends on the source of image data)
            delete[] buffer;

            ++input_path_it;

            continue;
        }


        // at this point the `jpeg_decoder` object has been assigned with the current frame buffer

        // not interested in very small bounding boxes (optional filter-out)
        const uint min_bbox_size = 16;

        // not interested in bounding boxes larger than half of frame height (optional filter-out)
        const uint max_bbox_size = downscaled_height / 2;

        // set boundaries for extending non-square bounding boxes into squares
        const BoundingBox frame_boundaries(0, 0, downscaled_width, downscaled_height);
        
        // process individual "movements" one by one
        uint movement_counter = 0;
        while (auto bbox = motion_detector.get_bounding_box()) {

            // detected area can be a non-square rectangle, extend it into a square if possible
            bbox.expand_to_square(frame_boundaries);

            // ignore bounding boxes outside of specified size range (optional filter-out)
            if (bbox.width() < min_bbox_size || bbox.width() > max_bbox_size) {

                continue;
            }

            // use bounding box info to specify which part of the frame to decode from original JPEG buffer
            jpeg_decoder.luma_decode(dest_buff, bbox, downscaling_block_writer);

            // this is where individual "movement" image decoded into `dest_buff` can be processed
            // (for testing/example purposes just write it to disk)
            std::filesystem::path output_path = output_dir / input_path_it->stem();
            output_path += "_" + std::to_string(movement_counter++) + ".pgm";
            mdjpeg_test_utils::write_as_pgm(output_path, dest_buff, dest_width, dest_height);
        }

        // update reference frame using the current input image
        motion_detector.set_reference(buffer, size);

        // clean up after use of image data (depends on the source of image data)
        delete[] buffer;

        ++input_path_it;
    }

    return 0;
}
