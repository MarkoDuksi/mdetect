#include "transform.h"

#include <cassert>

#include "Kernel.h"
#include "Image.h"


namespace {

const Kernel flat_8x8_kernel(1, 8, 8, 0, 0);
const Kernel flat_9x9_kernel(1, 9, 9, 4, 4);

}  // anonymous namespace


void transform::absdiff(Image& image, const Image& other) {

    assert(image.width == other.width && image.height == other.height &&
        "Element-wise abbsdiff cannot operate on differently sized images");

    for (uint32_t idx = 0; idx < image.size; ++idx) {

        image.data()[idx] = std::abs(image.data()[idx] - other.data()[idx]);
    }
}

void transform::threshold(Image& image, const uint8_t threshold) noexcept {

    for (uint32_t idx = 0; idx < image.size; ++idx) {

        image.data()[idx] = (image.data()[idx] <= threshold) ? 0 : 255;
    }
}

void transform::downscale_8x8(const Image& src_image, Image& dst_image) {

    auto postprocess = [](int x) -> uint8_t { return x / 64; };
    flat_8x8_kernel.convolve(src_image, 8, 8, postprocess, dst_image);
}

void transform::dilate_9x9(const Image& src_image, Image& dst_image) {

    auto postprocess = [](int x) -> uint8_t { return !x ? 0 : 255; };
    flat_9x9_kernel.convolve(src_image, 1, 1, postprocess, dst_image);
}
