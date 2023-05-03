#pragma once

#include <stdint.h>
#include <vector>

#include "image.h"


#define PADDING_VALUE_0 (uint8_t)0
#define PADDING_SIZE_1 (uint8_t)1


namespace bbox {

    class BBox {
        private:
            uint16_t m_topleft_X;
            uint16_t m_topleft_Y;
            uint16_t m_bottomright_X;
            uint16_t m_bottomright_Y;

        public:
            BBox (const uint16_t topleft_X, const uint16_t topleft_Y, const uint16_t bottomright_X, const uint16_t bottomright_Y);

            uint16_t width() const noexcept;
            uint16_t height() const noexcept;

            uint16_t topleft_X() const noexcept;
            uint16_t topleft_Y() const noexcept;
            uint16_t bottomright_X() const noexcept;
            uint16_t bottomright_Y() const noexcept;

            void merge(const BBox& other);
    };

    std::vector<BBox> get_bboxes(const Image& img, const uint16_t min_dimension, Image& aux_img);

}   // namespace bbox
