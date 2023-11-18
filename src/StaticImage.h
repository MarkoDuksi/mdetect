#pragma once

#include <stdint.h>

#include "Image.h"


template<uint16_t IMG_WIDTH, uint16_t IMG_HEIGHT>
class StaticImage : public Image {

    public:

        StaticImage() noexcept :
            Image(&m_internal_buff[0], IMG_WIDTH, IMG_HEIGHT)
            {}
        
    private:

        uint8_t m_internal_buff[IMG_WIDTH * IMG_HEIGHT] {};
};
