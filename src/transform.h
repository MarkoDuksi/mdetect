#pragma once

#include <stdint.h>

class Image;

namespace mdetect_transform {

void absdiff(Image& dst, const Image& src1, const Image& src2) noexcept;

void threshold(Image& dst, const Image& src, uint8_t threshold) noexcept;

void dilate(Image& dst, const Image& src, uint8_t str_elem_size) noexcept;

}   // namespace mdetect_transform
