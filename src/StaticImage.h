#pragma once

#include <stdint.h>

#include "Image.h"


/// \brief Lightweight static storage class with Image functionalities.
///
/// \tparam IMG_WIDTH   Image width in pixels.
/// \tparam IMG_HEIGHT  Image height in pixels.
///
/// The amount of storage provided is `IMG_WIDTH * IMG_HEIGHT`.
template<uint16_t IMG_WIDTH, uint16_t IMG_HEIGHT>
class StaticImage : public Image {

    public:

        StaticImage() noexcept :
            Image(&m_internal_buff[0], IMG_WIDTH, IMG_HEIGHT)
            {}

    private:

        uint8_t m_internal_buff[IMG_WIDTH * IMG_HEIGHT] {};
};
