#pragma once

#include <stdint.h>
#include <vector>

#include "image.h"
#include "transformations.h"
#include "bbox.h"

#define M_MIN_BBOX_DIMENSION 16


template<uint16_t static_img_width, uint16_t static_img_height>
class MotionDetector {
    private:
        Image m_reference_img;
        StaticImage<static_img_width, static_img_height> m_aux_img1;
        StaticImage<static_img_width, static_img_height> m_aux_img2;

    public:
        MotionDetector(Image& img_reference) noexcept :
            m_reference_img(img_reference)
            {}

        std::vector<bbox::BBox> detect(const Image& img) {
            // img.save("../output/input_orig.jpg");

            m_aux_img1 = img;
            // m_aux_img1.save("../output/input_copy.jpg");

            transformations::absdiff(m_aux_img1, m_reference_img);
            transformations::threshold(m_aux_img1, 127);
            // m_aux_img1.save("../output/input_copy_absdiff_threshold.jpg");

            // m_reference_img.save("../output/reference_orig.jpg");
            m_reference_img = img;
            // m_reference_img.save("../output/reference_new_from_input.jpg");

            transformations::dilate_8x8(m_aux_img1, m_aux_img2);
            // m_aux_img2.save("../output/input_copy_absdiff_threshold_convolve.jpg");

            return bbox::get_bboxes(m_aux_img2, M_MIN_BBOX_DIMENSION, m_aux_img1);
        }
};
