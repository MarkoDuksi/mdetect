#pragma once

#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <new>
#include <memory>

#include "mdjpeg.h"

#include "Image.h"
#include "transform.h"


union LabeledBBox {

    BoundingBox bbox {};

    struct {

        uint16_t _padding;
        uint16_t root_label;
        uint16_t is_root_node;
    } merge_rec;
};

template <uint8_t MAX_BBOXES_COUNT>
class CoreMotionDetector {

    public:

        CoreMotionDetector() = default;
        virtual ~CoreMotionDetector() = default;
        CoreMotionDetector(const CoreMotionDetector& other) = delete;
        CoreMotionDetector& operator=(const CoreMotionDetector& other) = delete;
        CoreMotionDetector(CoreMotionDetector&& other) = delete;
        CoreMotionDetector& operator=(CoreMotionDetector&& other) = delete;

        uint detect(uint8_t* const curr_frame_buff,
                    uint8_t* const ref_frame_buff,
                    uint8_t* const aux1_frame_buff,
                    uint8_t* const aux2_frame_buff,
                    uint8_t* const joint_bbox_buff,
                    const size_t   joint_bbox_buff_size,
                    const uint16_t frame_width,
                    const uint16_t frame_height,
                    const uint8_t  threshold,
                    const uint8_t  granularity) noexcept {

            const Image curr_img(curr_frame_buff, frame_width, frame_height);
            const Image ref_img(ref_frame_buff, frame_width, frame_height);
            Image aux1_img(aux1_frame_buff, frame_width, frame_height);
            Image aux2_img(aux2_frame_buff, frame_width, frame_height);

            // calculate pixel-wise absolute difference between frames
            mdetect_transform::absdiff(aux1_img, curr_img, ref_img);

            // posterize to 1-bit using custom threshold value
            mdetect_transform::threshold(aux1_img, aux1_img, threshold);

            // dilate with square block of size `granularity` x `granularity`
            mdetect_transform::dilate(aux2_img, aux1_img, granularity);

            // set individual buffers from joint bounding boxes buffer
            const uint capacity = set_bbox_buffers(joint_bbox_buff, joint_bbox_buff_size);

            // set default img value for probing beyond its boundaries
            const uint8_t padding_value = 0;

            // labeling of areas with detected "movements" starts with 1
            uint next_label = 1;

            // fit a tight bounding box around each "movement" area
            // (boxed connected components)
            for (int row = 0; (row < aux2_img.height) && next_label < capacity; ++row) {

                for (int col = 0; (col < aux2_img.width) && next_label < capacity; ++col) {

                    // if the current pixel value is non-zero
                    if (aux2_img.at(row, col)) {

                        const uint8_t W_label = aux2_img.at(row, col - 1, padding_value);

                        uint8_t N_label = aux2_img.at(row - 1, col, padding_value);

                        // resolve `N_label` to its root node label
                        if (!m_bboxes[N_label].merge_rec.is_root_node) {

                            N_label = m_bboxes[N_label].merge_rec.root_label;
                        }

                        // if W label is non-zero
                        if (W_label) {

                            // if N label is non-zero and different to W label
                            if (N_label && (N_label != W_label)) {

                                const auto [smaller, larger] = std::minmax(N_label, W_label);

                                // assign smaller label to the current pixel
                                aux2_img.at(row, col) = smaller;

                                // grow smaller label bounding box over the larger label one
                                m_bboxes[smaller].bbox.merge(m_bboxes[larger].bbox);

                                // set smaller label to be the larger label's root node
                                m_bboxes[larger].merge_rec.is_root_node = false;
                                m_bboxes[larger].merge_rec.root_label = smaller;
                           }

                            // ignore the N label
                            else {

                                // assign W label to the current pixel
                                aux2_img.at(row, col) = W_label;

                                // grow W label bounding box over the current pixel
                                m_bboxes[W_label].bbox.merge(BoundingBox(col, row));
                            }
                        }

                        // ignore the W label
                        else if (N_label) {

                            // assign N label to the current pixel
                            aux2_img.at(row, col) = N_label;

                            // grow N label bounding box over the current pixel
                            m_bboxes[N_label].bbox.merge(BoundingBox(col, row));
                        }

                        // else both W and N neighbours are zero -> create new bounding box
                        else {

                            // define a new bounding box for the current pixel
                            m_bboxes[next_label].bbox = BoundingBox(col, row);

                            // assign a new label to the current pixel
                            aux2_img.at(row, col) = next_label;

                            ++next_label;
                        }
                    }
                }
            }

            // reset bounding box counter used by `get_bounding_box`
            m_next_bbox_idx = 0;

            // copy bounding boxes with root node labels to private storage buffer
            m_stored_bbox_count = store_valid_bboxes(next_label);

            return m_stored_bbox_count;
        }

        BoundingBox get_bounding_box() noexcept {

            BoundingBox next_bbox;

            if (m_next_bbox_idx < m_stored_bbox_count) {

                next_bbox = m_bboxes_buff[m_next_bbox_idx++];
            }

            else {

                m_next_bbox_idx = 0;
            }

            return next_bbox;
        }

    protected:

        LabeledBBox* m_bboxes {nullptr};
        uint8_t m_next_bbox_idx {};
        uint8_t m_stored_bbox_count {};
        BoundingBox m_bboxes_buff[MAX_BBOXES_COUNT] {};

        uint set_bbox_buffers(uint8_t* joint_bbox_buff, size_t joint_bbox_buff_size) noexcept {

            // amount of bytes by which `joint_bbox_buff` is missaligned for storing `LabeledBBox` type
            const size_t missalignment = reinterpret_cast<uintptr_t>(joint_bbox_buff) % alignof(LabeledBBox);

            // aligned buffer capacity in terms of max allowed number of elements
            const uint aligned_capacity = (joint_bbox_buff_size - missalignment) / sizeof(LabeledBBox);

            // NOTE THE DETAILS BELOW:
            //   - useful capacity *cannot* exceed 256 elements by design (256 * 8 = 2048 bytes)
            //   - more memory than that, even if available through `joint_bbox_buff`, is not used
            //   - the limit is not arbitrary, it is the result of reusing image framebuffer which,
            //     being of type `uint8_t`, can differentiate between at most 256 labels
            //   - nevertheless, the typical use case (example) does not need more than 20 (160 bytes)
            const uint useful_capacity = std::min(256U, aligned_capacity);

            // alignment obvious to the compiler
            void* aligned_buff = reinterpret_cast<void*>(joint_bbox_buff);
            if(!std::align(alignof(LabeledBBox), useful_capacity, aligned_buff, joint_bbox_buff_size)) {

                return 0;
            }

            m_bboxes = reinterpret_cast<LabeledBBox*>(aligned_buff);
            LabeledBBox* elem_ptr = m_bboxes;

            // begin lifetime
            for (uint idx = 0; idx < useful_capacity; ++idx) {

                new (elem_ptr) LabeledBBox {};

                ++elem_ptr;
            }

            return useful_capacity;
        }

        virtual uint store_valid_bboxes(uint higher_bound) noexcept {

            uint dst_idx = 0;

            for (uint src_idx = 1; src_idx < higher_bound && dst_idx < MAX_BBOXES_COUNT; ++src_idx) {

                if (m_bboxes[src_idx].merge_rec.is_root_node) {

                    m_bboxes_buff[dst_idx++] = m_bboxes[src_idx].bbox;
                }
            }

            return dst_idx;
        }
};
