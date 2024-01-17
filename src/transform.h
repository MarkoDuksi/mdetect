#pragma once

#include <stdint.h>

class Image;

namespace mdetect_transform {

/// \brief Calculates element-wise absolute difference between two images.
///
/// \param dst   Image for writing output to.
/// \param src1  First input image.
/// \param src2  Second input image.
///
/// If references to destination and one of the source images alias the same
/// image, the aliased image will serve both as input and output.
void absdiff(Image& dst, const Image& src1, const Image& src2) noexcept;

/// \brief Binarizes image values either to \c 0 or \c UINT8_MAX.
///
/// \param dst        Image for writing output to.
/// \param src        Image to binarize according to `threshold`.
/// \param threshold  Inclusive upper limit on input for setting it to \c 0;
///                   exclusive lower limit on input for setting it to \c UINT8_MAX
///
/// If references to destination and source images alias the same image, the
/// operation will be done in place.
void threshold(Image& dst, const Image& src, uint8_t threshold) noexcept;

/// \brief Dilates a b/w image using a flat square-shaped structuring element.
///
/// \param dst            Image for writing output to.
/// \param src            Image to dilate.
/// \param str_elem_size  Width (and height) of the structuring element.
///
/// \attention Source and destination images **must not alias the same exact**
/// frame buffer. If memory savings are paramount, consult
/// JpegMotionDetector::detect in JpegMotionDetector.h on how to minimally
/// offset the frame buffers in order to avoid writing output over still
/// relevant input.
void dilate(Image& dst, const Image& src, uint8_t str_elem_size) noexcept;

}  // namespace mdetect_transform
