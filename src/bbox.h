#pragma once

#include <utility>
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

            static void setImgWidth(const size_t img_width);

            BBox (size_t idx);

            std::pair<size_t, size_t> getSize() const;

            void merge(size_t idx);

            void merge(BBox other);

    };

    template<typename T>
    std::vector<BBox> get_bboxes(const T* image, const size_t img_width, const size_t img_height, T* img_aux) {
        size_t idx = 0;
        size_t north_idx;
        size_t west_idx;

        size_t west_label;
        size_t north_label;
        size_t next_label = 1;

        BBox::setImgWidth(img_width);
        std::vector<BBox> bboxes_in_progress = { 0 };
        std::vector<size_t> labels_progression = { 0 };

        // first element
        if (image[idx]) {
            img_aux[idx] = next_label;
            bboxes_in_progress.emplace_back(idx);
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
                    bboxes_in_progress[west_label].merge(idx);
                }
                else {
                    img_aux[idx] = next_label;
                    bboxes_in_progress.emplace_back(idx);
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
                    bboxes_in_progress[north_label].merge(idx);
                }
                else {
                    img_aux[idx] = next_label;
                    bboxes_in_progress.emplace_back(idx);
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
                        bboxes_in_progress[west_label].merge(idx);
                        if (north_label) {
                            bboxes_in_progress[west_label].merge(bboxes_in_progress[north_label]);
                            labels_progression[north_label] = west_label;
                        }
                    }
                    else if (north_label) {
                        img_aux[idx] = north_label;
                        bboxes_in_progress[north_label].merge(idx);
                    }
                    else {
                        img_aux[idx] = next_label;
                        bboxes_in_progress.emplace_back(idx);
                        labels_progression.push_back(next_label++);
                    }
                }
                else
                    img_aux[idx] = 0;
            }
        }

        std::vector<BBox> bboxes_final;
        for (size_t i = 1; i < labels_progression.size(); ++i) {
            if (i == labels_progression[i]) {
                std::pair<size_t, size_t> bbox_size = bboxes_in_progress[i].getSize();
                if (std::min(bbox_size.first, bbox_size.second) >= 30)
                    bboxes_final.push_back(bboxes_in_progress[i]);
            }
        }

        return bboxes_final;
    }

}
