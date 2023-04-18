#include "bbox.h"

#include <stdint.h>
#include <algorithm>


using namespace bbox;

// private:
uint16_t BBox::s_img_width = 0;

// public:
BBox::BBox(const uint32_t idx, const uint16_t width, const uint16_t height) {
    const uint16_t row = idx / s_img_width;
    const uint16_t col = idx - row * s_img_width;
    topleft_X = col;
    topleft_Y = row;
    bottomright_X = col + width;
    bottomright_Y =  row + height;
}

// public:
void BBox::setImgWidth(const uint16_t img_width) {
    s_img_width = img_width;
}

// public:
uint16_t BBox::width() const {
    return bottomright_X - topleft_X;
}

// public:
uint16_t BBox::height() const {
    return bottomright_Y - topleft_Y;
}

// public:
void BBox::merge(const uint32_t idx) {
    uint16_t row = idx / s_img_width;
    uint16_t col = idx - row * s_img_width;
    bottomright_X = std::max(bottomright_X, (uint16_t)(col + 1));
    bottomright_Y = std::max(bottomright_Y, (uint16_t)(row + 1));
}

// public:
void BBox::merge(const BBox other) {
    topleft_X = std::min(topleft_X, other.topleft_X);
    topleft_Y = std::min(topleft_Y, other.topleft_Y);
    bottomright_X = std::max(bottomright_X, other.bottomright_X);
    bottomright_Y = std::max(bottomright_Y, other.bottomright_Y);
}
