#include "test-utils.h"

#include <utility>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>

#include <stdint.h>
#include <sys/types.h>


std::vector<std::filesystem::path> mdetect_test_utils::get_input_img_paths(const std::filesystem::path& input_dir) {

    std::vector<std::filesystem::path> input_paths;

    for (const auto& dir_entry : std::filesystem::directory_iterator(input_dir)){

        if (dir_entry.path().extension() == ".jpg") {

            input_paths.push_back(dir_entry.path());
        }
    }

    std::sort(input_paths.begin(), input_paths.end());

    return input_paths;
}

std::pair<uint8_t*, size_t> mdetect_test_utils::read_raw_jpeg_from_file(const std::filesystem::path& file_path) {

    std::ifstream file(file_path, std::ios::in | std::ios::binary | std::ios::ate);

    if (!file.is_open()) {

        return {nullptr, 0};
    }

    const auto size = file.tellg();
    char* raw_jpeg = new char[size];

    file.seekg(0);
    file.read(raw_jpeg, size);

    if (file.gcount() != size) {

        file.close();

        return {nullptr, 0};
    }

    file.close();

    return {reinterpret_cast<uint8_t*>(raw_jpeg), size};
}
