#pragma once

#include <vector>
#include "filters.h"

#define THRESHOLD_127 (uint8_t)127
#define PADDING_0 (uint8_t)0


template<typename T>
void update_background(T* img_foreground, size_t src_width, const size_t src_height, T* img_background) {
    const size_t img_size = src_width * src_height;
    memcpy(img_background, img_foreground, img_size * sizeof(T));
}

struct BBox {
    static size_t s_img_width;

    size_t topleft_X;
    size_t topleft_Y;
    size_t bottomright_X;
    size_t bottomright_Y;

    BBox (size_t idx) {
        size_t row = idx / s_img_width;
        size_t col = idx - row * s_img_width;

        topleft_X = col;
        topleft_Y = row;
        bottomright_X = col + 1;
        bottomright_Y =  row + 1;
    }

    std::pair<size_t, size_t> get_size() const {
        return {bottomright_X - topleft_X, bottomright_Y - topleft_Y};
    }

    void merge(size_t idx) {
        size_t row = idx / s_img_width;
        size_t col = idx - row * s_img_width;

        // topleft_X = std::min(topleft_X, col);
        // topleft_Y = std::min(topleft_Y, row);
        bottomright_X = std::max(bottomright_X, col + 1);
        bottomright_Y = std::max(bottomright_Y, row + 1);
    }

    void merge(BBox other) {
        topleft_X = std::min(topleft_X, other.topleft_X);
        topleft_Y = std::min(topleft_Y, other.topleft_Y);
        bottomright_X = std::max(bottomright_X, other.bottomright_X);
        bottomright_Y = std::max(bottomright_Y, other.bottomright_Y);
    }
};

size_t BBox::s_img_width;

template<typename T>
std::vector<BBox> get_bboxes(const T* image, const size_t img_width, const size_t img_height, T* scratchpad) {
    size_t idx = 0;
    size_t north_idx;
    size_t west_idx;

    size_t west_label;
    size_t north_label;
    size_t next_label = 1;

    BBox::s_img_width = img_width;
    std::vector<BBox> bboxes_in_progress = { 0 };
    std::vector<size_t> labels_progression = { 0 };

    // first element
    if (image[idx]) {
        scratchpad[idx] = next_label;
        bboxes_in_progress.emplace_back(idx);
        labels_progression.push_back(next_label++);
    } else scratchpad[idx] = 0;
    west_idx = idx++;

    // the rest of row 0
    while (idx < img_width) {
        if (image[idx]) {
            west_label = scratchpad[west_idx];
            if (west_label) {
                scratchpad[idx] = west_label;
                bboxes_in_progress[west_label].merge(idx);
            }
            else {
                scratchpad[idx] = next_label;
                bboxes_in_progress.emplace_back(idx);
                labels_progression.push_back(next_label++);
            }
        } else scratchpad[idx] = 0;
        west_idx = idx++;
    }

    // the rest of the image
    north_idx = 0;
    for (size_t i = 1; i < img_height; ++i) {
        // first element of row i
        if (image[idx]) {
            north_label = scratchpad[north_idx];
            north_label = labels_progression[north_label];
            if (north_label) {
                scratchpad[idx] = north_label;
                bboxes_in_progress[north_label].merge(idx);
            }
            else {
                scratchpad[idx] = next_label;
                bboxes_in_progress.emplace_back(idx);
                labels_progression.push_back(next_label++);
            }
        } else scratchpad[idx] = 0;
        west_idx = idx++;
        ++north_idx;
    
        // the rest of row i
        for (size_t j = 1; j < img_width; ++j, ++idx, ++west_idx, ++north_idx) {
            if (image[idx]) {
                west_label = scratchpad[west_idx];
                north_label = scratchpad[north_idx];
                north_label = labels_progression[north_label];
                if (west_label) {
                    scratchpad[idx] = west_label;
                    bboxes_in_progress[west_label].merge(idx);
                    if (north_label) {
                        bboxes_in_progress[west_label].merge(bboxes_in_progress[north_label]);
                        labels_progression[north_label] = west_label;
                    }
                }
                else if (north_label) {
                    scratchpad[idx] = north_label;
                    bboxes_in_progress[north_label].merge(idx);
                }
                else {
                    scratchpad[idx] = next_label;
                    bboxes_in_progress.emplace_back(idx);
                    labels_progression.push_back(next_label++);
                }
            }
            else scratchpad[idx] = 0;
        }
    }

    std::vector<BBox> bboxes_final;
    for (size_t i = 1; i < labels_progression.size(); ++i) {
        if (i == labels_progression[i]) {
            BBox bbox = bboxes_in_progress[i];
            std::pair<size_t, size_t> bbox_size = bbox.get_size();
            if (std::min(bbox_size.first, bbox_size.second) >= 32) {
                bboxes_final.push_back(bbox);
            }
        }
    }

    return bboxes_final;
}

template<typename T, size_t WIDTH, size_t HEIGHT>
class MotionDetector {
    private:
        size_t m_img_width;
        size_t m_img_height;
        T* m_img_ref;
        T* m_img_input;
        T* m_img_diff;
        T* m_img_binarized;
        T* m_img_aux;
        T* m_img_dilated;
        T* m_img_labeled;

    public:
        MotionDetector() {
                m_img_width = WIDTH / 4;
                m_img_height = HEIGHT / 4;

                size_t nominal_size = m_img_width * m_img_height;
                size_t padded_size = (m_img_width + 12) * (m_img_height + 12);

                m_img_ref = new T[nominal_size];
                m_img_input = new T[nominal_size];
                m_img_diff = new T[nominal_size];
                m_img_binarized = new T[nominal_size];
                m_img_aux = new T[padded_size];
                m_img_dilated = new T[nominal_size];
                m_img_labeled = new T[nominal_size];
            }

        void SetReference(const T* img) {
            filters::downscale(img, WIDTH, HEIGHT, m_img_ref);
        }

        std::vector<BBox> Detect(const T* img) {
            filters::downscale(img, WIDTH, HEIGHT, m_img_input);
            filters::absdiff(m_img_input, m_img_ref, m_img_width, m_img_height, m_img_diff);
            filters::threshold(m_img_diff, m_img_width, m_img_height, THRESHOLD_127, m_img_binarized);
            filters::pad(m_img_binarized, m_img_width, m_img_height, PADDING_0, 6, m_img_aux);
            filters::dilate(m_img_aux, m_img_width + 12, m_img_height + 12, m_img_dilated);
            update_background(m_img_input, m_img_width, m_img_height, m_img_ref);

            return get_bboxes(m_img_dilated, m_img_width, m_img_height, m_img_labeled);
        }
};
