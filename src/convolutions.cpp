#include "convolutions.h"


static const float flat_4x4_kernel_elements[] {
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1
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


static const convolutions::Kernel flat_4x4_kernel(flat_4x4_kernel_elements, 4, 4, 0, 0);
static const convolutions::Kernel round_13x13_kernel(round_13x13_kernel_elements, 13, 13, 6, 6);

void convolutions::downscale_4x4(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) { return static_cast<uint8_t>(x / 16); };
    convolve<float>(src_image, flat_4x4_kernel, 4, 4, postprocess, dst_image);
}

void convolutions::dilate_13x13(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) -> uint8_t { return !x ? 0 : 255; };
    convolve<float>(src_image, round_13x13_kernel, 1, 1, postprocess, dst_image);
}

void convolutions::erode_13x13(const Image& src_image, Image& dst_image) {
    auto postprocess = [](float x) -> uint8_t { return x < 120 * 255 ? 0 : 255; };
    convolve<float>(src_image, round_13x13_kernel, 1, 1, postprocess, dst_image);
}
