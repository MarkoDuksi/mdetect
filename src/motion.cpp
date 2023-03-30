#include <vector>
#include <memory>
#include <limits>
#include <cstring>
#include "filters.h"
#include "bbox.h"
#include "motion.h"

#define THRESHOLD_127 (uint8_t)127
#define PADDING_VALUE_0 (uint8_t)0
#define PADDING_SIZE_6 (size_t)6


void MotionDetector::updateReference(const uint8_t* newReference) {
    memcpy(m_reference, newReference, m_working_size * sizeof(uint8_t));
}

MotionDetector::MotionDetector(const uint8_t* init_img_reference, const size_t img_width, const size_t img_height, uint8_t* scratchpad) :
    m_full_width(img_width),
    m_full_height(img_height),
    m_working_width(m_full_width / 4),
    m_working_height(m_full_height / 4),
    m_working_size(m_working_width * m_working_height),
    m_padded_width(m_working_width + 2 * PADDING_SIZE_6),
    m_padded_height(m_working_height + 2 * PADDING_SIZE_6),
    m_padded_size(m_padded_width * m_padded_height)
    {
        if (scratchpad) {
            m_reference = scratchpad;               // required size: m_working_size
        } else {
            m_scratchpad = std::unique_ptr<uint8_t>(new uint8_t[3 * m_working_size + m_padded_size]);
            m_reference = m_scratchpad.get();       // required size: m_working_size
        }

        m_buff1 = m_reference + m_working_size;     // required size: m_working_size
        m_buff2 = m_buff1 + m_working_size;         // required size: m_working_size
        m_buff3 = m_buff2 + m_working_size;         // required size: m_padded_size

        setReference(init_img_reference);
    }

void MotionDetector::setReference(const uint8_t* init_img_reference) {
    filters::downscale(init_img_reference, m_full_width, m_full_height, m_reference);
}

std::vector<bbox::BBox> MotionDetector::detect(const uint8_t* img) {
    filters::downscale(img, m_full_width, m_full_height, m_buff1);
    filters::absdiff(m_buff1, m_reference, m_working_width, m_working_height, m_buff2);
    filters::threshold(m_buff2, m_working_width, m_working_height, THRESHOLD_127, m_buff2);
    filters::pad(m_buff2, m_working_width, m_working_height, PADDING_VALUE_0, PADDING_SIZE_6, m_buff3);
    filters::dilate(m_buff3, m_padded_width, m_padded_height, m_buff2);

    updateReference(m_buff1);

    return bbox::get_bboxes(m_buff2, m_working_width, m_working_height, m_buff3);
}