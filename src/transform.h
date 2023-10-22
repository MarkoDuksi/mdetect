#pragma once

#include <stdint.h>

class Image;

namespace mdetect_transform {

void absdiff(Image& image, const Image& other);

void threshold(Image& image, const uint8_t threshold) noexcept;

void dilate_13x13(const Image& src_image, Image& dst_image);

}   // namespace mdetect_transform
