#pragma once

#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <new>
#include <memory>

#include "mdjpeg.h"

#include "Image.h"
#include "transform.h"


/// \brief Low-level motion detection class.
///
/// \tparam MAX_BBOXES_COUNT  The maximum number of bounding boxes to store.
///
/// Implements low-level motion detection functionality based on inter-frame
/// comparison. Because its interface is highly parametrized it is not intended
/// for end-users but rather provided for customization by inheriting subclasses
/// (i.e. JpegMotionDetector). Uses a fixed-size internal storage for bounding
/// boxes for detected movements.
template <uint8_t MAX_BBOXES_COUNT>
class CoreMotionDetector {

    public:

        CoreMotionDetector() = default;
        virtual ~CoreMotionDetector() = default;
        CoreMotionDetector(const CoreMotionDetector& other) = delete;
        CoreMotionDetector& operator=(const CoreMotionDetector& other) = delete;
        CoreMotionDetector(CoreMotionDetector&& other) = delete;
        CoreMotionDetector& operator=(CoreMotionDetector&& other) = delete;

    protected:

        /// \brief Compares two frame buffers and detects movement regions between them.
        ///
        /// \param image1_frame_buff     Frame buffer of the first image (of size `frame_width * frame_height` bytes).
        /// \param image2_frame_buff     Frame buffer of the second image (of size `frame_width * frame_height` bytes).
        /// \param aux1_frame_buff       First auxiliary frame buffer (of size `frame_width * frame_height` bytes). Can alias one of the above.
        /// \param aux2_frame_buff       Second auxiliary frame buffer (of size `frame_width * frame_height` bytes).
        /// \param bbox_buff             A buffer needed in computing the bounding boxes.
        /// \param bbox_buff_size        Size of the `bbox_buff` in bytes.
        /// \param frame_width           Width of the image frame in pixels.
        /// \param frame_height          Height of the image frame in pixels.
        /// \param threshold             Minimum absolute value for a change in pixel intensity between frames to be considered as due to movement.
        /// \param granularity           Level of detail for movements mask. Determines the minimum distance separating distinct submasks, as well as the padding around them.
        /// \return                      Total count of movement regions detected.
        ///
        /// Detected regions are internally stored as bounding boxes which can
        /// be retrieved by calling get_bounding_box(). Calling detect() resets
        /// any previously stored bounding boxes.
        ///
        /// \par Choosing the size for \c bbox_buff
        /// A temporary storage for bounding boxes (at 8 bytes per bounding box)
        /// is needed for detection. At most 256 bounding boxes can be used
        /// which amounts to 2048 bytes + alignment provisions (max 8 bytes).
        /// The rest, if provided, cannot be used. However, much less than that
        /// is normally needed. The simplest recommendation is to just reuse the
        /// `aux1_frame_buff` or otherwise (advanced, see implementation of
        /// JpegMotionDetector::detect) just the part of it not reused by
        /// `aux2_frame_buff`. If detected movement regions seem excessively
        /// detailed or incomplete, increase the buffer size and/or
        /// \c granularity.
        uint detect(uint8_t* const image1_frame_buff,
                    uint8_t* const image2_frame_buff,
                    uint8_t* const aux1_frame_buff,
                    uint8_t* const aux2_frame_buff,
                    uint8_t* const bbox_buff,
                    const size_t   bbox_buff_size,
                    const uint16_t frame_width,
                    const uint16_t frame_height,
                    const uint8_t  threshold,
                    const uint8_t  granularity) noexcept {

            const Image curr_img(image1_frame_buff, frame_width, frame_height);
            const Image ref_img(image2_frame_buff, frame_width, frame_height);
            Image aux1_img(aux1_frame_buff, frame_width, frame_height);
            Image aux2_img(aux2_frame_buff, frame_width, frame_height);

            // calculate pixel-wise absolute difference between frames
            mdetect_transform::absdiff(aux1_img, curr_img, ref_img);

            // posterize to 1-bit using custom threshold value
            mdetect_transform::threshold(aux1_img, aux1_img, threshold);

            // dilate with square block of size `granularity` x `granularity`
            mdetect_transform::dilate(aux2_img, aux1_img, granularity);

            // set temporary bounding box buffer
            const uint capacity = set_bbox_buffer(bbox_buff, bbox_buff_size);

            // set default img value for probing beyond its boundaries
            const uint8_t padding_value = 0;

            // labeling of areas with detected "movements" starts with 1
            uint next_label = 1;

            // fit a tight (after dilation) bounding box around each "movement" area
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

                        // else both W and N neighbors are zero -> create new bounding box
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

        /// \brief Retrieves the next bounding box from store
        ///
        /// \return  A bounding box around the next detected movement or a null-box after the last one.
        ///
        /// If no movements are detected, always returns a null-box.
        /// Otherwise, after the sentinel, restarts with the first box.
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

    private:

        // sets `m_bboxes` to correctly aligned address within provided buffer
        // returns the capacity in number of elements
        uint set_bbox_buffer(uint8_t* bbox_buff, size_t bbox_buff_size) noexcept {

            // amount of bytes by which `bbox_buff` is misaligned for storing `LabeledBBox` type
            const size_t misalignment = reinterpret_cast<uintptr_t>(bbox_buff) % alignof(LabeledBBox);

            // aligned buffer capacity in terms of max allowed number of elements
            const uint aligned_capacity = (bbox_buff_size - misalignment) / sizeof(LabeledBBox);

            // NOTE THE DETAILS BELOW:
            //   - useful capacity *cannot* exceed 256 elements by design (256 * 8 = 2048 bytes)
            //   - more memory than that, even if available through `bbox_buff`, is not used
            //   - the limit is not arbitrary, it is the result of reusing image frame buffer which,
            //     being of type `uint8_t`, can differentiate between at most 256 labels
            //   - nevertheless, the typical use case (example) does not need more than 20 (160 bytes)
            const uint useful_capacity = std::min(256U, aligned_capacity);

            // alignment obvious to the compiler
            void* aligned_buff = reinterpret_cast<void*>(bbox_buff);
            if(!std::align(alignof(LabeledBBox), useful_capacity, aligned_buff, bbox_buff_size)) {

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

        // internally stores valid bounding boxes from temporary buffer for
        // subsequent retrieval by `get_bounding_box`
        virtual uint store_valid_bboxes(uint higher_bound) noexcept {

            uint dst_idx = 0;

            for (uint src_idx = 1; src_idx < higher_bound && dst_idx < MAX_BBOXES_COUNT; ++src_idx) {

                if (m_bboxes[src_idx].merge_rec.is_root_node) {

                    m_bboxes_buff[dst_idx++] = m_bboxes[src_idx].bbox;
                }
            }

            return dst_idx;
        }

        // bounding box tagged with a merge record used by `detect` to track the
        // growing bounding boxes
        union LabeledBBox {

            BoundingBox bbox {};

            struct {

                uint16_t _padding;
                uint16_t root_label;
                uint16_t is_root_node;
            } merge_rec;
        };

    protected:

        LabeledBBox* m_bboxes {nullptr};
        uint8_t m_next_bbox_idx {};
        uint8_t m_stored_bbox_count {};
        BoundingBox m_bboxes_buff[MAX_BBOXES_COUNT] {};
};
