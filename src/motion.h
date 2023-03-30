#pragma once


class MotionDetector {
    private:
        const size_t m_full_width;
        const size_t m_full_height;
        const size_t m_working_width;
        const size_t m_working_height;
        const size_t m_working_size;
        const size_t m_padded_width;
        const size_t m_padded_height;
        const size_t m_padded_size;
        std::unique_ptr<uint8_t> m_scratchpad;
        uint8_t* m_reference;
        uint8_t* m_buff1;
        uint8_t* m_buff2;
        uint8_t* m_buff3;

        void updateReference(const uint8_t* newReference);

    public:
        MotionDetector(const uint8_t* init_img_reference, const size_t img_width, const size_t img_height, uint8_t* scratchpad = nullptr);

        void setReference(const uint8_t* init_img_reference);

        std::vector<bbox::BBox> detect(const uint8_t* img);
};
