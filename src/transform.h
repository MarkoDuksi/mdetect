#pragma once

#include <stdint.h>

class Image;

namespace transform {

void absdiff(Image& image, const Image& other);

void threshold(Image& image, const uint8_t threshold) noexcept;

void downscale_8x8(const Image& src_image, Image& dst_image);

void dilate_9x9(const Image& src_image, Image& dst_image);

}   // namespace transform
