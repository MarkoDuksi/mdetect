#pragma once

#include <stdint.h>
#include <cassert>
#include <algorithm>

#include "Image.h"


struct BoundingBox {

    public:

        uint16_t topleft_X {};
        uint16_t topleft_Y {};
        uint16_t bottomright_X {};
        uint16_t bottomright_Y {};

        BoundingBox (const uint16_t topleft_X = 0, const uint16_t topleft_Y = 0, const uint16_t bottomright_X = 0, const uint16_t bottomright_Y = 0) noexcept :
            topleft_X(topleft_X),
            topleft_Y(topleft_Y),
            bottomright_X(bottomright_X ? bottomright_X : topleft_X + 1),
            bottomright_Y(bottomright_Y ? bottomright_Y : topleft_Y + 1)
            {
                assert (this->bottomright_X != 0 && "BoundingBox out of bounds (bottomright_X overflow)");
                assert (this->bottomright_Y != 0 && "BoundingBox out of bounds (bottomright_Y overflow)");
                assert (this->topleft_X < this->bottomright_X && "BoundingBox out of bounds (topleft_X >= bottomleft_X)");
                assert (this->topleft_Y < this->bottomright_Y && "BoundingBox out of bounds (topleft_Y >= bottomleft_Y)");
            }

        bool operator>(const BoundingBox& other) const noexcept {

            return std::min(width(), height()) > std::min(other.width(), other.height());
        }

        uint16_t width() const noexcept {

            return bottomright_X - topleft_X;
        }

        uint16_t height() const noexcept {

            return bottomright_Y - topleft_Y;
        }

        void merge(const BoundingBox& other) noexcept {

            topleft_X = std::min(topleft_X, other.topleft_X);
            topleft_Y = std::min(topleft_Y, other.topleft_Y);

            bottomright_X = std::max(bottomright_X, other.bottomright_X);
            bottomright_Y = std::max(bottomright_Y, other.bottomright_Y);
        }

        bool expand_to_square(const BoundingBox& outer_bounds) {

            // if expanding would grow the bounding box outside of `outer_bounds`
            if (std::max(width(), height()) > std::min(outer_bounds.width(), outer_bounds.height())) {

                return false;
            }

            // if width should match height
            if (width() < height()) {

                topleft_X = std::max(0, topleft_X - static_cast<int>((height() - width()) / 2.0f + 0.5f));
                bottomright_X = std::min(static_cast<int>(outer_bounds.bottomright_X), topleft_X + height());
                topleft_X = bottomright_X - height();
            }

            // if height should match width
            else if (height() < width()) {

                topleft_Y = std::max(0, topleft_Y - static_cast<int>((width() - height()) / 2.0f + 0.5f));
                bottomright_Y = std::min(static_cast<int>(outer_bounds.bottomright_Y), topleft_Y + width());
                topleft_Y = bottomright_Y - width();
            }

            return true;
        }
};
