#pragma once

#include <utility>
#include <vector>
#include <filesystem>

#include <stdint.h>
#include <sys/types.h>


namespace mdetect_test_utils {

std::vector<std::filesystem::path> get_input_img_paths(const std::filesystem::path& input_dir);

std::pair<uint8_t*, size_t> read_raw_jpeg_from_file(const std::filesystem::path& file_path);

}  // namespace mdetect_test_utils
