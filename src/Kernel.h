#pragma once

#include <stdint.h>

#include "Image.h"


/// \brief Class for performing kernel convolutions with images.
///
/// \tparam T  Type used for kernel elements and accumulator buffer.
///
/// Type \c T must be set with consideration to kernel size and kernel elements
/// values such that the accumulator buffer of that type does not overflow
/// during multiply-accumulate with image having all pixel values set to
/// \c UINT8_MAX.

template<typename T>
class Kernel {

    public:

        /// \brief Defines heterogeneous kernel.
        ///
        /// \param elements     Array of element values of size `width * height`.
        /// \param width        Width in number of elements.
        /// \param height       Height in number of elements.
        /// \param anchor_X     X-coordinate (column index) of the anchor element (kernel origin).
        /// \param anchor_Y     Y-coordinate (row index) of the anchor element (kernel origin).
        /// \param stride_X     Step size in X direction when performing convolution.
        /// \param stride_Y     Step size in Y direction when performing convolution.
        /// \param postprocess  Custom transformation when performing convolution.
        Kernel(const T* const elements,
               const uint8_t width,
               const uint8_t height,
               const uint8_t anchor_X,
               const uint8_t anchor_Y,
               const uint8_t stride_X,
               const uint8_t stride_Y,
               uint8_t (*postprocess)(T)) noexcept :
            m_elements(elements),
            m_postprocess(postprocess),
            m_width(width),
            m_height(height),
            m_anchor_X(anchor_X),
            m_anchor_Y(anchor_Y),
            m_stride_X(stride_X),
            m_stride_Y(stride_Y)
            {}

        /// \brief Defines homogeneous (flat) kernel.
        ///
        /// \param repeating_element  A single value for all `width * height` elements.
        /// \param width              Width in number of elements.
        /// \param height             Height in number of elements.
        /// \param anchor_X           X-coordinate (column index) of the anchor element (kernel origin).
        /// \param anchor_Y           Y-coordinate (row index) of the anchor element (kernel origin).
        /// \param stride_X           Step size in X direction when performing convolution.
        /// \param stride_Y           Step size in Y direction when performing convolution.
        /// \param postprocess        Custom transformation when performing convolution.
        ///
        /// There are no behavioral differences between homogeneous and
        /// heterogeneous kernels. The only difference is their storage size. A
        /// homogeneous kernel does not store an array of all its elements but
        /// rather a single value for the same repeating element entirely
        /// filling its matrix.
        Kernel(const T repeating_element,
               const uint8_t width,
               const uint8_t height,
               const uint8_t anchor_X,
               const uint8_t anchor_Y,
               const uint8_t stride_X,
               const uint8_t stride_Y,
               uint8_t (*postprocess)(T)) noexcept :
            Kernel(&m_repeating_element, width, height, anchor_X, anchor_Y, stride_X, stride_Y, postprocess)
            {
                m_repeating_element = repeating_element;
                m_is_heterogeneous = false;
            }

        /// \brief Convolves the kernel with an image.
        ///
        /// \param dst        Image for writing output to.
        /// \param src        Image to convolve the kernel with.
        /// \param pad_value  Value to pad the image with.
        ///
        /// Each output pixel value is obtained in a two-step process. First
        /// step is a multiply-accumulate between kernel elements and the
        /// overlayed image elements (image is considered padded by
        /// \c pad_value). Second step is custom postprocessing of the
        /// accumulated value (e.g. normalization, applying dilation or erosion
        /// logic...).
        ///
        /// When calculating the value of `(X, Y)` output pixel the kernel is
        /// positioned over the source image so that its anchor element (origin)
        /// is at `(stride_X * X, stride_Y * Y)` in the source image reference
        /// frame.
        void convolve(Image& dst, const Image& src, const uint8_t pad_value = 0) const noexcept {

            for (uint dst_row = 0, src_row = 0; dst_row < dst.height; ++dst_row, src_row += m_stride_Y) {

                for (uint dst_col = 0, src_col = 0; dst_col < dst.width; ++dst_col, src_col += m_stride_X) {

                    dst.at(dst_row, dst_col) = m_postprocess(stamp(src, src_row, src_col, pad_value));
                }
            }
        }

    private:

        const T* const m_elements {nullptr};
        uint8_t (*m_postprocess)(T) {nullptr};
        T m_repeating_element {};
        bool m_is_heterogeneous {true};

        const uint8_t m_width {};
        const uint8_t m_height {};
        const uint8_t m_anchor_X {};
        const uint8_t m_anchor_Y {};
        const uint8_t m_stride_X {};
        const uint8_t m_stride_Y {};

        // multiply-accumulate with kernel positioned over the image at `(img_col, img_row)`
        T stamp(const Image& image, const uint16_t img_row, const uint16_t img_col, const uint8_t pad_value) const {

            T accumulator = 0;

            // start with last kernel element
            uint16_t kernel_idx = m_is_heterogeneous * (m_width * m_height - 1);

            // set the Y-coordinate of the top-left corner of the sliding window
            int32_t row = static_cast<int32_t>(img_row) - static_cast<int32_t>(m_anchor_Y);
            for (uint window_Y = 0; window_Y < m_height; ++window_Y, ++row) {

                // set the X-coordinate of the top-left corner of the sliding window
                int32_t col = static_cast<int32_t>(img_col) - static_cast<int32_t>(m_anchor_X);
                for (uint window_X = 0; window_X < m_width; ++window_X, ++col) {

                    accumulator += (m_elements[kernel_idx] * image.at(row, col, pad_value));

                    // decrement to previous kernel element unless dealing with homogeneous kernel
                    kernel_idx -= m_is_heterogeneous;
                }
            }

            return accumulator;
        }
};
