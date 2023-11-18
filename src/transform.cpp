#include "transform.h"

#include <cmath>

#include "Kernel.h"
#include "Image.h"


void mdetect_transform::absdiff(Image &dst, const Image &src1, const Image &src2) noexcept {

    for (uint32_t idx = 0; idx < dst.width * dst.height; ++idx) {

        dst.data()[idx] = std::abs(src1.data()[idx] - src2.data()[idx]);
    }
}

void mdetect_transform::threshold(Image& dst, const Image& src, const uint8_t threshold) noexcept {

    for (uint32_t idx = 0; idx < dst.width * dst.height; ++idx) {

        dst.data()[idx] = (src.data()[idx] <= threshold) ? 0 : 255;
    }
}

void mdetect_transform::dilate(Image& dst, const Image& src, const uint8_t struct_elem_size) noexcept {

    const Kernel<int> dilate_kernel(
        1,                     // single (repeating) element
        struct_elem_size,      // height in px
        struct_elem_size,      // width in px
        struct_elem_size / 2,  // anchor X-coordinate
        struct_elem_size / 2,  // anchor Y-coordinate
        1,                     // stride in X direction
        1,                     // stride in Y direction
        [](int x) noexcept -> uint8_t { return !x ? 0 : 255; }  // dilation-specific postprocessing
    );

    dilate_kernel.convolve(dst, src);
}
