#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "bbox.h"


class MotionDetector {
    private:
        const uint16_t m_img_full_width;
        const uint16_t m_img_full_height;
        const uint16_t m_min_bbox_dimension;
        const uint16_t m_working_width;
        const uint16_t m_working_height;
        const uint32_t m_working_size;
        const uint16_t m_padded_width;
        const uint16_t m_padded_height;
        const uint32_t m_padded_size;
        std::unique_ptr<uint8_t> m_scratchpad;
        uint8_t* m_reference;
        uint8_t* m_aux_buff1;
        uint8_t* m_aux_buff2;
        uint8_t* m_aux_buff3;

        void updateReference(const uint8_t* new_reference);

    public:
        MotionDetector(const uint16_t img_width, const uint16_t img_height, const uint16_t min_bbox_dimension = 1, const uint8_t* init_img_reference = nullptr, uint8_t* scratchpad = nullptr);

        void setBlankReference();

        void setImgReference(const uint8_t* img_reference);

        std::vector<bbox::BBox> detect(const uint8_t* img);
};
