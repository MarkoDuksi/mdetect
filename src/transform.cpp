#include "transform.h"

#include <cassert>

#include "Kernel.h"
#include "Image.h"


namespace {

const Kernel flat_13x13_kernel(1, 13, 13, 6, 6);

}  // anonymous namespace


void mdetect_transform::absdiff(Image& image, const Image& other) {

    assert(image.width == other.width && image.height == other.height &&
        "Element-wise abbsdiff cannot operate on differently sized images");

    for (uint32_t idx = 0; idx < image.size; ++idx) {

        image.data()[idx] = std::abs(image.data()[idx] - other.data()[idx]);
    }
}

void mdetect_transform::threshold(Image& image, const uint8_t threshold) noexcept {

    for (uint32_t idx = 0; idx < image.size; ++idx) {

        image.data()[idx] = (image.data()[idx] <= threshold) ? 0 : 255;
    }
}

void mdetect_transform::dilate_13x13(const Image& src_image, Image& dst_image) {

    auto postprocess = [](int x) -> uint8_t { return !x ? 0 : 255; };
    flat_13x13_kernel.convolve(src_image, 1, 1, postprocess, dst_image);
}
