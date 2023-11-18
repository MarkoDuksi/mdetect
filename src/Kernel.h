#pragma once

#include <stdint.h>

#include "Image.h"


template<typename T>
class Kernel {

    public:

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
                m_has_single_repeating_element = true;
            }

        void convolve(Image& dst, const Image& src) const noexcept {

            for (uint dst_row = 0, src_row = 0; dst_row < dst.height; ++dst_row, src_row += m_stride_Y) {

                for (uint dst_col = 0, src_col = 0; dst_col < dst.width; ++dst_col, src_col += m_stride_X) {

                    dst.at(dst_row, dst_col) = m_postprocess(stamp(src, src_row, src_col));
                }
            }
        }

    private:

        const T* const m_elements {nullptr};
        uint8_t (*m_postprocess)(T) {nullptr};
        T m_repeating_element {};
        bool m_has_single_repeating_element {false};

        const uint8_t m_width {};
        const uint8_t m_height {};
        const uint8_t m_anchor_X {};
        const uint8_t m_anchor_Y {};
        const uint8_t m_stride_X {};
        const uint8_t m_stride_Y {};

        T stamp(const Image& image, const uint16_t img_row, const uint16_t img_col) const {

            T accumulator = 0;
            uint16_t kernel_idx = !m_has_single_repeating_element * (m_width * m_height - 1);

            int32_t row = static_cast<int32_t>(img_row) - static_cast<int32_t>(m_anchor_Y);
            for (uint window_Y = 0; window_Y < m_height; ++window_Y, ++row) {

                int32_t col = static_cast<int32_t>(img_col) - static_cast<int32_t>(m_anchor_X);
                for (uint window_X = 0; window_X < m_width; ++window_X, ++col) {

                    accumulator += (m_elements[kernel_idx] * image.at(row, col, 0));
                    kernel_idx -= !m_has_single_repeating_element;
                }
            }

            return accumulator;
        }
};
