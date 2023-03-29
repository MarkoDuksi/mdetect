#include <cstdlib>
#include "bbox.h"


using namespace bbox;

size_t BBox::s_img_width = 0;

void BBox::setImgWidth(size_t img_width) {
    s_img_width = img_width;
}

bbox::BBox::BBox (size_t idx) {
    size_t row = idx / s_img_width;
    size_t col = idx - row * s_img_width;
    topleft_X = col;
    topleft_Y = row;
    bottomright_X = col + 1;
    bottomright_Y =  row + 1;
}

std::pair<size_t, size_t> BBox::get_size() const {
    return {bottomright_X - topleft_X, bottomright_Y - topleft_Y};
}

void BBox::merge(size_t idx) {
    size_t row = idx / s_img_width;
    size_t col = idx - row * s_img_width;
    // topleft_X = std::min(topleft_X, col);
    // topleft_Y = std::min(topleft_Y, row);
    bottomright_X = std::max(bottomright_X, col + 1);
    bottomright_Y = std::max(bottomright_Y, row + 1);
}

void BBox::merge(BBox other) {
    topleft_X = std::min(topleft_X, other.topleft_X);
    topleft_Y = std::min(topleft_Y, other.topleft_Y);
    bottomright_X = std::max(bottomright_X, other.bottomright_X);
    bottomright_Y = std::max(bottomright_Y, other.bottomright_Y);
}
