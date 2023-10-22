#pragma once

#include <stdint.h>
#include <array>
#include <optional>
#include <algorithm>

#include "Image.h"
#include "BoundingBox.h"
#include "transform.h"


template<uint16_t IMG_WIDTH, uint16_t IMG_HEIGHT>
class MotionDetector {

    public:

        MotionDetector(const Image& img_reference) noexcept {

            m_reference_img = img_reference;
        }

        void set_reference(const Image& img_reference) {

            m_reference_img = img_reference;
        }

        void detect(const Image& img) {

            m_mask = img;

            mdetect_transform::absdiff(m_mask, m_reference_img);
            mdetect_transform::threshold(m_mask, 127);
            mdetect_transform::dilate_13x13(m_mask, m_aux);

            m_bboxes.fill(BoundingBox());
            m_labels_lookup.fill(0);

            const uint8_t padding_value = 0;

            uint8_t next_label = 1;

            for (uint16_t row = 0; (row < m_aux.height) && next_label; ++row) {

                for (uint16_t col = 0; (col < m_aux.width) && next_label; ++col) {

                    // if the current pixel value is non-zero
                    if (m_aux.at(row, col)) {

                        const uint8_t W_label = m_aux.at(row, col - 1, padding_value);

                        uint8_t N_label = m_aux.at(row - 1, col, padding_value);

                        // resolve N_label to its parent label
                        N_label = m_labels_lookup[N_label];

                        // if W label is non-zero
                        if (W_label) {

                            // if N label is non-zero and different to W label
                            if (N_label && (N_label != W_label)) {

                                const auto [smaller, larger] = std::minmax(N_label, W_label);

                                // assign smaller label to the current pixel
                                m_aux.at(row, col) = smaller;

                                // make larger label resolve to smaller label as its parent
                                m_labels_lookup[larger] = smaller;

                                // grow smaller label bbox over the larger label one
                                m_bboxes[smaller].merge(m_bboxes[larger]);
                            }

                            // ignore the N label
                            else {

                                // assign W label to the current pixel
                                m_aux.at(row, col) = W_label;

                                // grow W label bbox over the current pixel
                                m_bboxes[W_label].merge(BoundingBox(col, row));
                            }
                        }

                        // ignore the W label
                        else if (N_label) {

                            // assign N label to the current pixel
                            m_aux.at(row, col) = N_label;

                            // grow N label bbox over the current pixel
                            m_bboxes[N_label].merge(BoundingBox(col, row));
                        }

                        // else both W and N neighbours are zero -> create new bbox
                        else {

                            // define a new bbox for the current pixel
                            m_bboxes[next_label] = BoundingBox(col, row);

                            // assign a new label to the current pixel
                            m_aux.at(row, col) = next_label;

                            // add the label to LUT
                            m_labels_lookup[next_label] = next_label;

                            ++next_label;
                        }
                    }
                    // else the current pixel value is zero -> nothing to do
                }
            }

            // sort m_mboxes by smaller dimension in descending order (insertion sort)
            BoundingBox temp;

            for (int i = 1; i < next_label; ++i) {

                // do not consider labels that do not point to themselves in the lookup table
                if (m_labels_lookup[i] != i) {

                    temp = BoundingBox();
                }

                else {

                    temp = m_bboxes[i];
                }

                int j;
                for (j = i; j > 0 && temp > m_bboxes[j - 1]; --j) {

                    m_bboxes[j] = m_bboxes[j - 1];
                }
                m_bboxes[j] = temp;
            }
        }

        const Image& get_mask() {

            return m_mask;
        }

        std::optional<BoundingBox> get_bounding_box() {

            if (std::min(m_bboxes[m_next_bbox].height(), m_bboxes[m_next_bbox].width()) > 1) {

                return m_bboxes[m_next_bbox++];
            }

            else {

                m_next_bbox = 0;

                return {};
            }
        }

    private:

        StaticImage<IMG_WIDTH, IMG_HEIGHT> m_reference_img;
        StaticImage<IMG_WIDTH, IMG_HEIGHT> m_aux;
        StaticImage<IMG_WIDTH, IMG_HEIGHT> m_mask;

        std::array<uint8_t, 256> m_labels_lookup;
        std::array<BoundingBox, 256> m_bboxes;

        uint8_t m_next_bbox {};
};
