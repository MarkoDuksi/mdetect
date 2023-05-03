#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "bbox.h"
#include "image.h"


class MotionDetector {
    private:
        Image m_reference_img;
        Image m_aux_img1;
        Image m_aux_img2;

    public:
        MotionDetector(Image&& img_reference, uint8_t* const scratchpad) noexcept;

        std::vector<bbox::BBox> detect(const Image& img);
};
