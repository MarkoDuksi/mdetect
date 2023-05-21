#pragma once

#include <stdint.h>
#include <vector>

#include "image.h"
#include "transform.h"
#include "bbox.h"

#define M_MIN_BBOX_DIMENSION 16


template<uint16_t static_img_width, uint16_t static_img_height>
class MotionDetector {
    private:
        StaticImage<static_img_width, static_img_height> m_reference_img;
        StaticImage<static_img_width, static_img_height> m_aux_img1;
        StaticImage<static_img_width, static_img_height> m_aux_img2;

    public:
        MotionDetector(Image& img_reference) noexcept :
            m_reference_img(img_reference)
            {}

        std::vector<bbox::BBox> detect(const Image& img) {
            m_aux_img1 = img;

            transform::absdiff(m_aux_img1, m_reference_img);
            transform::threshold(m_aux_img1, 127);

            m_reference_img = img;

            transform::dilate_8x8(m_aux_img1, m_aux_img2);

            return bbox::get_bboxes(m_aux_img2, M_MIN_BBOX_DIMENSION, m_aux_img2);
        }
};
