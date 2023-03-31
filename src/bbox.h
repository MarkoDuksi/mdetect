#pragma once

#include <vector>


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
        size_t idx = 0;
        size_t north_idx;
        size_t west_idx;

        size_t west_label;
        size_t north_label;
        size_t next_label = 1;

        BBox::setImgWidth(img_width);
        std::vector<BBox> bboxes = { 0 };
        std::vector<size_t> labels_progression = { 0 };

        // first element
        if (image[idx]) {
            img_aux[idx] = next_label;
            bboxes.emplace_back(idx);
            labels_progression.push_back(next_label++);
        } else
            img_aux[idx] = 0;
        west_idx = idx++;

        // the rest of row 0
        while (idx < img_width) {
            if (image[idx]) {
                west_label = img_aux[west_idx];
                if (west_label) {
                    img_aux[idx] = west_label;
                    bboxes[west_label].merge(idx);
                }
                else {
                    img_aux[idx] = next_label;
                    bboxes.emplace_back(idx);
                    labels_progression.push_back(next_label++);
                }
            } else
                img_aux[idx] = 0;
            west_idx = idx++;
        }

        // the rest of the image
        north_idx = 0;
        for (size_t i = 1; i < img_height; ++i) {
            // first element of row i
            if (image[idx]) {
                north_label = img_aux[north_idx];
                north_label = labels_progression[north_label];
                if (north_label) {
                    img_aux[idx] = north_label;
                    bboxes[north_label].merge(idx);
                }
                else {
                    img_aux[idx] = next_label;
                    bboxes.emplace_back(idx);
                    labels_progression.push_back(next_label++);
                }
            } else
                img_aux[idx] = 0;
            west_idx = idx++;
            ++north_idx;
        
            // the rest of row i
            for (size_t j = 1; j < img_width; ++j, ++idx, ++west_idx, ++north_idx) {
                if (image[idx]) {
                    west_label = img_aux[west_idx];
                    north_label = img_aux[north_idx];
                    north_label = labels_progression[north_label];
                    if (west_label) {
                        img_aux[idx] = west_label;
                        bboxes[west_label].merge(idx);
                        if (north_label) {
                            bboxes[west_label].merge(bboxes[north_label]);
                            labels_progression[north_label] = west_label;
                        }
                    }
                    else if (north_label) {
                        img_aux[idx] = north_label;
                        bboxes[north_label].merge(idx);
                    }
                    else {
                        img_aux[idx] = next_label;
                        bboxes.emplace_back(idx);
                        labels_progression.push_back(next_label++);
                    }
                }
                else
                    img_aux[idx] = 0;
            }
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
