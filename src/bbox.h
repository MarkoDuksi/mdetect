#pragma once

#include <cstdint>
#include <vector>
#include "filters.h"

#define PADDING_VALUE_0 (uint8_t)0
#define PADDING_SIZE_1 (size_t)1


namespace bbox {

    struct BBox {
        private:
            static size_t s_img_width;

        public:
            size_t topleft_X;
            size_t topleft_Y;
            size_t bottomright_X;
            size_t bottomright_Y;

            BBox (const size_t idx, const size_t width = 1, const size_t height = 1);

            static void setImgWidth(const size_t img_width);

            size_t width() const;

            size_t height() const;

            void merge(const size_t idx);

            void merge(const BBox other);

    };

    template<typename T>
    std::vector<BBox> get_bboxes(const T* image, const size_t img_width, const size_t img_height, const size_t min_dimension, T* img_aux) {
        filters::pad(image, img_width, img_height, PADDING_VALUE_0, PADDING_SIZE_1, img_aux);

        size_t origin_idx = 0;
        size_t idx = img_width + 3;
        size_t north_idx = 1;
        size_t west_idx = idx - 1;

        size_t west_label;
        size_t north_label;
        size_t next_label = 1;

        BBox::setImgWidth(img_width);
        std::vector<BBox> bboxes = { 0 };
        std::vector<size_t> labels_progression = { 0 };

        for (size_t i = 0; i < img_height; ++i) {
            for (size_t j = 0; j < img_width; ++j, ++idx, ++origin_idx, ++west_idx, ++north_idx) {
                if (img_aux[idx]) {
                    west_label = img_aux[west_idx];
                    north_label = img_aux[north_idx];
                    north_label = labels_progression[north_label];
                    if (west_label) {
                        img_aux[idx] = west_label;
                        bboxes[west_label].merge(origin_idx);
                        if (north_label) {
                            bboxes[west_label].merge(bboxes[north_label]);
                            labels_progression[north_label] = west_label;
                        }
                    }
                    else if (north_label) {
                        img_aux[idx] = north_label;
                        bboxes[north_label].merge(origin_idx);
                    }
                    else {
                        img_aux[idx] = next_label;
                        bboxes.emplace_back(origin_idx);
                        labels_progression.push_back(next_label++);
                    }
                }
                else
                    img_aux[idx] = 0;
            }
            idx += 2;
            west_idx += 2;
            north_idx += 2;
        }

        std::vector<BBox> bboxes_filtered;
        for (size_t i = 1; i < labels_progression.size(); ++i) {
            if (i == labels_progression[i] && std::min(bboxes[i].width(), bboxes[i].height()) >= min_dimension)
                bboxes_filtered.push_back(bboxes[i]);
        }
        bboxes_filtered.shrink_to_fit();

        return bboxes_filtered;
    }

}
