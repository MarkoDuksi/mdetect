#include "motion.h"

#include <cstring>
#include <utility>

#include "convolutions.h"


#define M_MIN_BBOX_DIMENSION 30


// public:
MotionDetector::MotionDetector(Image&& img_reference, uint8_t* const scratchpad) noexcept :
    m_reference_img(std::move(img_reference)),
    m_aux_img1(scratchpad, m_reference_img.width(), m_reference_img.height()),
    m_aux_img2(scratchpad + m_reference_img.size(), m_reference_img.width(), m_reference_img.height())
    {}

// public:
std::vector<bbox::BBox> MotionDetector::detect(const Image& img) {
    // img.save("../output/input_orig.jpg");

    m_aux_img1 = img;
    // m_aux_img1.save("../output/input_copy.jpg");

    m_aux_img1.absdiff(m_reference_img).threshold(127);
    // m_aux_img1.save("../output/input_copy_absdiff_threshold.jpg");

    // m_reference_img.save("../output/reference_orig.jpg");
    m_reference_img = img;
    // m_reference_img.save("../output/reference_new_from_input.jpg");

    convolutions::dilate_13x13(m_aux_img1, m_aux_img2);
    // m_aux_img2.save("../output/input_copy_absdiff_threshold_convolve.jpg");

    return bbox::get_bboxes(m_aux_img2, M_MIN_BBOX_DIMENSION, m_aux_img1);
}
