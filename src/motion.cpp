#include "motion.h"

#include <cstring>

#include "filters.h"


#define THRESHOLD_127 (uint8_t)127
#define PADDING_VALUE_0 (uint8_t)0
#define PADDING_SIZE_6 (uint8_t)6


// private:
void MotionDetector::updateReference(const uint8_t* new_reference) {
    std::memcpy(m_reference, new_reference, m_working_size * sizeof(uint8_t));
}

// public:
MotionDetector::MotionDetector(const uint16_t img_width, const uint16_t img_height, const uint16_t min_bbox_dimension, const uint8_t* init_img_reference, uint8_t* scratchpad) :
    m_img_full_width(img_width),
    m_img_full_height(img_height),
    m_min_bbox_dimension(min_bbox_dimension),
    m_working_width(m_img_full_width / 4),
    m_working_height(m_img_full_height / 4),
    m_working_size(m_working_width * m_working_height),
    m_padded_width(m_working_width + 2 * PADDING_SIZE_6),
    m_padded_height(m_working_height + 2 * PADDING_SIZE_6),
    m_padded_size(m_padded_width * m_padded_height)
    {
        if (scratchpad)
            m_reference = scratchpad;                   // required size: m_working_size
        else {
            m_scratchpad = std::unique_ptr<uint8_t>(new uint8_t[3 * m_working_size + m_padded_size]);
            m_reference = m_scratchpad.get();           // required size: m_working_size
        }

        m_aux_buff1 = m_reference + m_working_size;     // required size: m_working_size
        m_aux_buff2 = m_aux_buff1 + m_working_size;     // required size: m_working_size
        m_aux_buff3 = m_aux_buff2 + m_working_size;     // required size: m_padded_size

        if (init_img_reference)
            setImgReference(init_img_reference);
        else
            setBlankReference();
    }

// public:
void MotionDetector::setBlankReference() {
    std::memset(m_reference, 0, m_working_size * sizeof(uint8_t));
}

// public:
void MotionDetector::setImgReference(const uint8_t* img_reference) {
    filters::downscale(img_reference, m_img_full_width, m_img_full_height, m_reference);
}

// public:
std::vector<bbox::BBox> MotionDetector::detect(const uint8_t* img) {
    filters::downscale(img, m_img_full_width, m_img_full_height, m_aux_buff1);
    filters::absdiff(m_aux_buff1, m_reference, m_working_width, m_working_height, m_aux_buff2);
    filters::threshold(m_aux_buff2, m_working_width, m_working_height, THRESHOLD_127, m_aux_buff2);
    filters::pad(m_aux_buff2, m_working_width, m_working_height, PADDING_VALUE_0, PADDING_SIZE_6, m_aux_buff3);
    filters::dilate(m_aux_buff3, m_padded_width, m_padded_height, m_aux_buff2);

    updateReference(m_aux_buff1);

    return bbox::get_bboxes(m_aux_buff2, m_working_width, m_working_height, m_min_bbox_dimension, m_aux_buff3);
}
