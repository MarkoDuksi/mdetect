#pragma once

#include <stdint.h>
#include <cassert>
#include <algorithm>

#include "Image.h"


struct BoundingBox {

    public:

        BoundingBox (const uint16_t topleft_X = 0, const uint16_t topleft_Y = 0, const uint16_t bottomright_X = 0, const uint16_t bottomright_Y = 0) noexcept :
            m_topleft_X(topleft_X),
            m_topleft_Y(topleft_Y),
            m_bottomright_X(bottomright_X ? bottomright_X : topleft_X + 1),
            m_bottomright_Y(bottomright_Y ? bottomright_Y : topleft_Y + 1)
            {
                assert (m_bottomright_X != 0 && "BoundingBox out of bounds (bottomright_X overflow)");
                assert (m_bottomright_Y != 0 && "BoundingBox out of bounds (bottomright_Y overflow)");
                assert (m_topleft_X < m_bottomright_X && "BoundingBox out of bounds (topleft_X >= bottomleft_X)");
                assert (m_topleft_Y < m_bottomright_Y && "BoundingBox out of bounds (topleft_Y >= bottomleft_Y)");
            }

        bool operator>(const BoundingBox& other) const noexcept {

            return std::min(width(), height()) > std::min(other.width(), other.height());
        }

        uint16_t width() const noexcept {

            return m_bottomright_X - m_topleft_X;
        }

        uint16_t height() const noexcept {

            return m_bottomright_Y - m_topleft_Y;
        }

        void merge(const BoundingBox& other) noexcept {

            m_topleft_X = std::min(m_topleft_X, other.m_topleft_X);
            m_topleft_Y = std::min(m_topleft_Y, other.m_topleft_Y);

            m_bottomright_X = std::max(m_bottomright_X, other.m_bottomright_X);
            m_bottomright_Y = std::max(m_bottomright_Y, other.m_bottomright_Y);
        }

        bool expand_to_square(const BoundingBox& outer_bounds) {

            // if expanding would grow the bounding box outside of `outer_bounds`
            if (std::max(width(), height()) > std::min(outer_bounds.width(), outer_bounds.height())) {

                return false;
            }

            // if width should match height
            if (width() < height()) {

                m_topleft_X = std::max(0, m_topleft_X - static_cast<int>((height() - width()) / 2.0f + 0.5f));
                m_bottomright_X = std::min(static_cast<int>(outer_bounds.m_bottomright_X), m_topleft_X + height());
                m_topleft_X = m_bottomright_X - height();
            }

            // if height should match width
            else if (height() < width()) {

                m_topleft_Y = std::max(0, m_topleft_Y - static_cast<int>((width() - height()) / 2.0f + 0.5f));
                m_bottomright_Y = std::min(static_cast<int>(outer_bounds.m_bottomright_Y), m_topleft_Y + width());
                m_topleft_Y = m_bottomright_Y - width();
            }

            return true;
        }

    private:

        uint16_t m_topleft_X {};
        uint16_t m_topleft_Y {};
        uint16_t m_bottomright_X {};
        uint16_t m_bottomright_Y {};

};
