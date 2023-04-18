#pragma once

#include <stdint.h>
#include <vector>

#include "filters.h"

#define PADDING_VALUE_0 (uint8_t)0
#define PADDING_SIZE_1 (uint8_t)1


namespace bbox {

    struct BBox {
        private:
            static uint16_t s_img_width;

        public:
            uint16_t topleft_X;
            uint16_t topleft_Y;
            uint16_t bottomright_X;
            uint16_t bottomright_Y;

            BBox (const uint32_t idx, const uint16_t width = 1, const uint16_t height = 1);

            static void setImgWidth(const uint16_t img_width);

            uint16_t width() const;
            uint16_t height() const;

            void merge(const uint32_t idx);
            void merge(const BBox other);

    };

    template<typename T>
    std::vector<BBox> get_bboxes(const T* image, const uint16_t img_width, const uint16_t img_height, const uint16_t min_dimension, T* aux_buff) {
        filters::pad(image, img_width, img_height, PADDING_VALUE_0, PADDING_SIZE_1, aux_buff);

        // data at image[origin_idx] corresponds to data at aux_buff[idx]
        uint32_t origin_idx = 0;
        uint32_t idx = img_width + 3;
        uint32_t north_idx = 1;
        uint32_t west_idx = idx - 1;

        uint32_t west_label;
        uint32_t north_label;
        uint32_t next_label = 1;

        BBox::setImgWidth(img_width);
        std::vector<BBox> bboxes { 0 };
        std::vector<uint32_t> labels_progression { 0 };

        for (size_t row = 0; row < img_height; ++row) {
            for (size_t col = 0; col < img_width; ++col, ++idx, ++origin_idx, ++west_idx, ++north_idx) {
                if (aux_buff[idx]) {
                    west_label = aux_buff[west_idx];
                    north_label = aux_buff[north_idx];
                    north_label = labels_progression[north_label];
                    if (west_label) {
                        aux_buff[idx] = west_label;
                        bboxes[west_label].merge(origin_idx);
                        if (north_label) {
                            bboxes[west_label].merge(bboxes[north_label]);
                            labels_progression[north_label] = west_label;
                        }
                    }
                    else if (north_label) {
                        aux_buff[idx] = north_label;
                        bboxes[north_label].merge(origin_idx);
                    }
                    else {
                        aux_buff[idx] = next_label;
                        bboxes.emplace_back(origin_idx);
                        labels_progression.push_back(next_label++);
                    }
                }
                else
                    aux_buff[idx] = 0;
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
