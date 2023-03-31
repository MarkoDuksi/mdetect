#include <cstdlib>
#include "bbox.h"


using namespace bbox;

// private:
size_t BBox::s_img_width = 0;

// public:
BBox::BBox(const size_t idx, const size_t width, const size_t height) {
    const size_t row = idx / s_img_width;
    const size_t col = idx - row * s_img_width;
    topleft_X = col;
    topleft_Y = row;
    bottomright_X = col + width;
    bottomright_Y =  row + height;
}

void BBox::setImgWidth(const size_t img_width) {
    s_img_width = img_width;
}

 size_t BBox::width() const {
    return bottomright_X - topleft_X;
}

 size_t BBox::height() const {
    return bottomright_Y - topleft_Y;
}

void BBox::merge(const size_t idx) {
    size_t row = idx / s_img_width;
    size_t col = idx - row * s_img_width;
    // topleft_X = std::min(topleft_X, col);
    // topleft_Y = std::min(topleft_Y, row);
    bottomright_X = std::max(bottomright_X, col + 1);
    bottomright_Y = std::max(bottomright_Y, row + 1);
}

void BBox::merge(const BBox other) {
    topleft_X = std::min(topleft_X, other.topleft_X);
    topleft_Y = std::min(topleft_Y, other.topleft_Y);
    bottomright_X = std::max(bottomright_X, other.bottomright_X);
    bottomright_Y = std::max(bottomright_Y, other.bottomright_Y);
}
