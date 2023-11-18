#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <algorithm>

#include "mdjpeg.h"

#include "CoreMotionDetector.h"


template<uint16_t FRAME_WIDTH,
         uint16_t FRAME_HEIGHT,
         uint8_t GRANULARITY = 1 + std::min(FRAME_WIDTH, FRAME_HEIGHT) / 8,
         uint8_t MAX_BBOXES_COUNT = 5>
class JpegMotionDetector : public CoreMotionDetector<MAX_BBOXES_COUNT> {

    public:

        explicit JpegMotionDetector(JpegDecoder& decoder) noexcept :
            m_decoder(&decoder)
            {}

        bool set_reference(const uint8_t* const ref_buff, const size_t size) noexcept {

            return decode_jpeg(m_ref_raw_buff, ref_buff, size);
        }

        int detect(const uint8_t* const frame_buff, const size_t size, const uint8_t threshold = 127) noexcept {

            // NOTE THE DETAILS BELOW:
            //  - at one point, "content" of `scratchpad1` will be dilated into `scratchpad2`
            //  - dilation operations *cannot* be done in place
            //  - the pointers would, normally, point to two separate, non-overlapping buffers
            //  - however, in this implementation the two buffers overlap(!) by design
            //  - this approach uses less memory and in a more cache friendly manner
            
            // to prevent read-write aliasing, set the smallest required pointer offset for
            // the greatest allowable buffer overlap
            constexpr uint offset = FRAME_WIDTH * (GRANULARITY / 2 + 1);

            // set the size for the single, joint buffer and allocate it on the stack
            constexpr uint32_t joint_buffer_size = FRAME_WIDTH * FRAME_HEIGHT + offset;
            uint8_t joint_buffer[joint_buffer_size];

            // set the starting points to overlapping memory blocks within the joint buffer

            // `scratchpad1` is used for:
            //  1) current frame pixel data
            //  2) element-wise abosolute difference of 1) and reference frame pixel data
            //  3) in place thresholded 2)
            uint8_t* const scratchpad1 = &joint_buffer[offset];

            // `scratchpad2` is used for dilated 3)
            uint8_t* const scratchpad2 = &joint_buffer[0];

            // `scratchpad3` is used for boxed connected components algorithm
            uint8_t* const scratchpad3 = &joint_buffer[FRAME_WIDTH * FRAME_HEIGHT];

            if (!decode_jpeg(scratchpad1, frame_buff, size)) {

                return -1;
            }

            // the `CoreMotionDetector::detect` is oblivious to the overlap
            return CoreMotionDetector<MAX_BBOXES_COUNT>::detect(m_ref_raw_buff,
                                                                scratchpad1,
                                                                scratchpad1,
                                                                scratchpad2,
                                                                scratchpad3,
                                                                offset,
                                                                FRAME_WIDTH,
                                                                FRAME_HEIGHT,
                                                                threshold,
                                                                GRANULARITY);
        }

    private:

        JpegDecoder* const m_decoder {nullptr};
        uint8_t m_ref_raw_buff[FRAME_WIDTH * FRAME_HEIGHT] {};

        bool decode_jpeg(uint8_t* const raw_buff, const uint8_t* const jpeg_buff, const size_t size) noexcept {

            if (!m_decoder->assign(jpeg_buff, size)) {

                return false;
            }

            // if decoding with no downscaling
            if (FRAME_WIDTH == m_decoder->get_width() && FRAME_HEIGHT == m_decoder->get_height()) {
                return m_decoder->luma_decode(raw_buff, {0, 0, FRAME_WIDTH, FRAME_HEIGHT});
            }

            // if downscaling exactly 8 times both horizontally and vertically
            if (FRAME_WIDTH * 8 == m_decoder->get_width() && FRAME_HEIGHT * 8 == m_decoder->get_height()) {

                return m_decoder->dc_luma_decode(raw_buff, {0, 0, FRAME_WIDTH, FRAME_HEIGHT});
            }

            // generic downscaling factor
            DownscalingBlockWriter<FRAME_WIDTH, FRAME_HEIGHT> downscaling_block_writer;

            return m_decoder->luma_decode(raw_buff, {0, 0, FRAME_WIDTH, FRAME_HEIGHT}, downscaling_block_writer);
        }
};
