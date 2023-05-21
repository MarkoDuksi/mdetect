#include "transform.h"


static const float flat_4x4_kernel_elements[] {
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1
};

static const float flat_8x8_kernel_elements[] {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1
};

static const float round_13x13_kernel_elements[] {
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0
};


static const transform::Kernel flat_4x4_kernel(flat_4x4_kernel_elements, 4, 4, 0, 0);
static const transform::Kernel flat_8x8_kernel(flat_8x8_kernel_elements, 8, 8, 0, 0);
static const transform::Kernel round_13x13_kernel(round_13x13_kernel_elements, 13, 13, 6, 6);

void transform::threshold(Image& image, const uint8_t threshold) noexcept {
    for (uint32_t idx = 0; idx < image.size; ++idx) {
        image.data()[idx] = image.data()[idx] <= threshold ? 0 : 255;
    }
}

void transform::absdiff(Image& image, const Image& other) {
    assert(image.width == other.width && image.height == other.height &&
        "Element-wise abbsdiff cannot operate on differently sized images");
    for (uint32_t idx = 0; idx < image.size; ++idx) {
        image.data()[idx] = std::abs(image.data()[idx] - other.data()[idx]);
    }
}

void transform::downscale_4x4(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) { return static_cast<uint8_t>(x / 16); };
    convolve<float>(src_image, flat_4x4_kernel, 4, 4, postprocess, dst_image);
}

void transform::downscale_8x8(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) { return static_cast<uint8_t>(x / 64); };
    convolve<float>(src_image, flat_8x8_kernel, 8, 8, postprocess, dst_image);
}

void transform::dilate_8x8(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) -> uint8_t { return !x ? 0 : 255; };
    convolve<float>(src_image, flat_8x8_kernel, 1, 1, postprocess, dst_image);
}

void transform::dilate_13x13(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) -> uint8_t { return !x ? 0 : 255; };
    convolve<float>(src_image, round_13x13_kernel, 1, 1, postprocess, dst_image);
}

void transform::erode_13x13(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) -> uint8_t { return x < 120 * 255 ? 0 : 255; };
    convolve<float>(src_image, round_13x13_kernel, 1, 1, postprocess, dst_image);
}
