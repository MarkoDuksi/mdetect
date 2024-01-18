#pragma once

#include <stdint.h>


namespace mdetect {

/// \brief Wrapper class around an 8-bit grayscale frame buffer.
///
/// Keeps record of image width and height and provides ways for (row, col)
/// 2D-indexing of pixel values. Intended for accessing an already existing
/// frame buffer or for extending via derived classes that manage storage
/// for their own frame buffer.
class Image {

    private:

        uint8_t* const m_data;

    public:

        const uint16_t width {};
        const uint16_t height {};

        Image(uint8_t* const ptr_data, const uint16_t width, const uint16_t height) noexcept :
            m_data(ptr_data),
            width(width),
            height(height)
            {}

        Image(const Image& other) = delete;
        Image& operator=(const Image& other) = delete;
        Image(Image&& other) = delete;
        Image& operator=(Image&& other) = delete;
        virtual ~Image() = default;

        /// \brief Const accessor for (row, col) 2D-indexing.
        ///
        /// \param row        Y-coordinate of pixel being accessed.
        /// \param col        X-coordinate of pixel being accessed.
        /// \return           Pixel value at (X, Y).
        ///
        /// Caller is responsible for ensuring (X, Y) is within image bounds.
        uint8_t at(const uint16_t row, const uint16_t col) const noexcept {

            return m_data[row * width + col];
        }

        /// \brief Non-const accessor for (row, col) 2D-indexing.
        ///
        /// \param row        Y-coordinate of pixel being accessed.
        /// \param col        X-coordinate of pixel being accessed.
        /// \return           Reference to pixel value at (X, Y).
        ///
        /// Caller is responsible for ensuring (X, Y) is within image bounds.
        uint8_t& at(const uint16_t row, const uint16_t col) noexcept {

            return m_data[row * width + col];
        }

        /// \brief Const accessor for padded (row, col) 2D-indexing.
        ///
        /// \param row        Y-coordinate of pixel being accessed.
        /// \param col        X-coordinate of pixel being accessed.
        /// \param pad_value  Value to return on out-of-bounds access.
        /// \return           Pixel value at (X, Y) if (X, Y) is within image bounds,
        ///                   \c pad_value otherwise.
        uint8_t at(const int32_t row, const int32_t col, const uint8_t pad_value) const noexcept {

            return (row >= 0 && col >= 0 && row < height && col < width) ?
                m_data[row * width + col] : pad_value;
        }
};

}  // namespace mdetect
