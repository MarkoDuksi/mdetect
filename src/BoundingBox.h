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

        uint16_t topleft_X() const noexcept {

            return m_topleft_X;
        }

        uint16_t topleft_Y() const noexcept {

            return m_topleft_Y;
        }

        uint16_t bottomright_X() const noexcept {

            return m_bottomright_X;
        }

        uint16_t bottomright_Y() const noexcept {

            return m_bottomright_Y;
        }

        void merge(const BoundingBox& other) {

            m_topleft_X = std::min(m_topleft_X, other.m_topleft_X);
            m_topleft_Y = std::min(m_topleft_Y, other.m_topleft_Y);

            m_bottomright_X = std::max(m_bottomright_X, other.m_bottomright_X);
            m_bottomright_Y = std::max(m_bottomright_Y, other.m_bottomright_Y);
        }

    private:

        uint16_t m_topleft_X {};
        uint16_t m_topleft_Y {};
        uint16_t m_bottomright_X {};
        uint16_t m_bottomright_Y {};
};
