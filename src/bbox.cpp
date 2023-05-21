#include "bbox.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>

#include "image.h"

#define PADDING_VALUE_0 (uint8_t)0


// public:
bbox::BBox::BBox (const uint16_t topleft_X = 0, const uint16_t topleft_Y = 0, const uint16_t bottomright_X = 0, const uint16_t bottomright_Y = 0) :
    m_topleft_X(topleft_X),
    m_topleft_Y(topleft_Y),
    m_bottomright_X(bottomright_X ? bottomright_X : topleft_X + 1),
    m_bottomright_Y(bottomright_Y ? bottomright_Y : topleft_Y + 1)
    {
        if (!m_bottomright_X) {
            throw std::overflow_error("BBox out of bounds (bottomright_X overflow)");
        }
        if (!m_bottomright_Y) {
            throw std::overflow_error("BBox out of bounds (bottomright_Y overflow)");
        }
        if (m_topleft_X >= m_bottomright_X) {
            throw std::invalid_argument("BBox out of bounds (topleft_X >= bottomleft_X)");
        }
        if (m_topleft_Y >= m_bottomright_Y) {
            throw std::invalid_argument("BBox out of bounds (topleft_Y >= bottomleft_Y)");
        }
    }

// public:
uint16_t bbox::BBox::width() const noexcept {
    return m_bottomright_X - m_topleft_X;
}

// public:
uint16_t bbox::BBox::height() const noexcept {
    return m_bottomright_Y - m_topleft_Y;
}

// public:
uint16_t bbox::BBox::topleft_X() const noexcept {
    return m_topleft_X;
}

// public:
uint16_t bbox::BBox::topleft_Y() const noexcept {
    return m_topleft_Y;
}

// public:
uint16_t bbox::BBox::bottomright_X() const noexcept {
    return m_bottomright_X;
}

// public:
uint16_t bbox::BBox::bottomright_Y() const noexcept {
    return m_bottomright_Y;
}

// public:
void bbox::BBox::merge(const BBox& other) {
    m_topleft_X = std::min(m_topleft_X, other.m_topleft_X);
    m_topleft_Y = std::min(m_topleft_Y, other.m_topleft_Y);
    m_bottomright_X = std::max(m_bottomright_X, other.m_bottomright_X);
    m_bottomright_Y = std::max(m_bottomright_Y, other.m_bottomright_Y);
}


std::vector<bbox::BBox> bbox::get_bboxes(const Image& img, const uint16_t min_dimension, Image& aux_img) {
    assert(img.width == aux_img.width && img.height == aux_img.height &&
           "bbox::get_bboxes requires same-sized aux_img for a scratchpad");

    aux_img = img;

    using namespace bbox;

    uint32_t next_label = 0;

    // dummy 0-th entry label
    std::vector<uint32_t> labels_lookup { next_label++ };
    // dummy 0-th entry bbox
    std::vector<BBox> bboxes { {0, 0} };

    for (uint16_t row = 0; row < aux_img.height; ++row) {
        for (uint16_t col = 0; col < aux_img.width; ++col) {
            // if the current pixel value is non-zero
            if (aux_img.at(row, col)) {
                const uint32_t& W_label = aux_img.at(row, col - 1, PADDING_VALUE_0);
                const uint32_t& N_label = aux_img.at(row - 1, col, PADDING_VALUE_0);
                const uint32_t& N_label_parent = labels_lookup[N_label];
                // if W label is non-zero
                if (W_label) {
                    // if N label's parent is non-zero and different to W label
                    if (N_label_parent && (N_label_parent != W_label)) {
                        const uint32_t& smaller = N_label_parent < W_label ? N_label_parent : W_label;
                        const uint32_t& larger = N_label_parent > W_label ? N_label_parent : W_label;
                        // assign smaller label to the current pixel
                        aux_img.at(row, col) = smaller;
                        // make larger label resolve to smaller label for its parent
                        labels_lookup[larger] = smaller;
                        // grow smaller label bbox over the larger label one
                        bboxes[smaller].merge(bboxes[larger]);
                    }
                    // else -> ignore the N label
                    else {
                        // assign W label to the current pixel
                        aux_img.at(row, col) = W_label;
                        // grow W label bbox over the current pixel
                        bboxes[W_label].merge(BBox(col, row));
                    }
                }
                // else if only N neighbour is non-zero -> ignore the W label
                else if (N_label_parent) {
                    // assign N label's parent to the current pixel
                    aux_img.at(row, col) = N_label_parent;
                    // grow N label's parent bbox over the current pixel
                    bboxes[N_label_parent].merge(BBox(col, row));
                }
                // else both W and N neighbours are zero -> create new bbox
                else {
                    // assign a new label to the current pixel
                    aux_img.at(row, col) = next_label;
                    // add a new bbox (around the current pixel) to the growing sequence
                    bboxes.emplace_back(col, row);
                    // add its label to the lookup table
                    labels_lookup.push_back(next_label++);
                }
            }
            // else the current pixel value is zero -> nothing to be done
        }
    }

    std::vector<BBox> bboxes_filtered;
    // ignore the 0-th entry dummy
    for (size_t i = 1; i < labels_lookup.size(); ++i) {
        // consider only labels that still point to themselves in the lookup table
        if (i == labels_lookup[i] &&
            // and bboxes that are at least min_dimension wide *and* tall
            std::min(bboxes[i].width(), bboxes[i].height()) >= min_dimension) {
                bboxes_filtered.push_back(bboxes[i]);
            }
    }
    bboxes_filtered.shrink_to_fit();

    return bboxes_filtered;
}
