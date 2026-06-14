/**
 * @file grayscale.ixx
 * @brief Declaration and definition of the @ref yellow_mug::GrayscaleProcessor class.
 */

export module yellow_mug.processors:grayscale;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Algorithm used to compute the grayscale intensity of a pixel.
 *
 * @see GrayscaleProcessor::m_method
 */
enum class GrayscaleMethod
{
	Luminosity, ///< Weighted sum (using BT.709 weights): `0.2126 R + 0.7152 G + 0.0722 B`.
	Average,    ///< Simple mean: `(R + G + B) / 3`.
	Lightness   ///< HSL lightness: `(max(R, G, B) + min(R, G, B)) / 2`.
};

/**
 * @brief Processor that converts a frame to grayscale.
 *
 * @details
 * Replaces the red, green, and blue channels of every pixel with a single
 * intensity value computed from the configurable @ref m_method, while
 * leaving the alpha channel unchanged.
 *
 * @see GrayscaleProcessorNode
 */
class GrayscaleProcessor : public Processor
{
public:
	/**
	 * @brief Converts the input frame to grayscale.
	 *
	 * @details
	 * Clones @p inputs `[0]` and, for each pixel, computes an intensity
	 * value according to @ref m_method using integer arithmetic, then
	 * writes that value to the red, green, and blue channels. The alpha
	 * channel is preserved.
	 *
	 * @param inputs Span containing one element; the frame to convert.
	 * @return The grayscale @ref Frame, or `nullptr` if @p inputs `[0]`
	 *         is `nullptr`.
	 */
	[[nodiscard]] std::shared_ptr<const Frame> operator()(
		const std::span<const std::shared_ptr<const Frame>> inputs) override
	{
		clear_error();
		if (!inputs[0])
		{
			set_error("Missing input frame.");
			return nullptr;
		}

		const Frame& input = *inputs[0];
		if (input.dimensions().empty())
			return std::make_shared<const Frame>(input.clone());

		auto out = input.clone();

		for (auto&& px : out.pixel_view())
		{
			const auto r{px[0]};
			const auto g{px[1]};
			const auto b{px[2]};

			std::uint8_t l{0};
			switch (m_method)
			{
			// For all cases this mathematical property is used:
			// round(x / y) = floor((x + floor(y / 2)) / y).
			case GrayscaleMethod::Luminosity:
			{
				// BT.709 uses weights:
				// - 0.2126 for R,
				// - 0.7152 for G,
				// - and 0.0722 for B.
				// Premultiply using the highest scale that avoids overflows to guarantee the best quality.
				static constexpr auto SHIFT{24u};
				static constexpr auto SCALE{0x1u << SHIFT};
				static constexpr auto R_WEIGHT = static_cast<std::uint32_t>(0.2126 * SCALE + 0.5);
				static constexpr auto G_WEIGHT = static_cast<std::uint32_t>(0.7152 * SCALE + 0.5);
				static constexpr auto B_WEIGHT = static_cast<std::uint32_t>(0.0722 * SCALE + 0.5);
				// R_WEIGHT + G_WEIGHT + B_WEIGHT may equal SCALE with uncertainty equal to 1
				// by the rounding-apportionment problem; each weight is independently optimal.
				// The induced luminance error is at most 3 * (0.5 / SCALE) * 255, which is
				// approximately 4.6e-5 — well below one uint8_t LSB.
				// static_assert(R_WEIGHT + G_WEIGHT + B_WEIGHT == SCALE);

				// x = r * R_WEIGHT + g * G_WEIGHT + b * B_WEIGHT
				// y = SCALE = 1 << SHIFT
				// floor(y / 2) = SCALE >> 1 (= 1 << (SHIFT - 1))
				// a / SCALE = a >> SHIFT
				// Assert that uint32_t won't overflow for white pixel.
				static_assert(255u * (R_WEIGHT + G_WEIGHT + B_WEIGHT) + (SCALE >> 1) <= std::numeric_limits<std::uint32_t>::max());
				l = static_cast<std::uint8_t>((r * R_WEIGHT + g * G_WEIGHT + b * B_WEIGHT + (SCALE >> 1)) >> SHIFT);
				break;
			}
			case GrayscaleMethod::Average:
			{
				// x = r + g + b
				// y = 3
				// floor(y / 2) = 1
				l = static_cast<std::uint8_t>((r + g + b + 1) / 3);
				break;
			}
			case GrayscaleMethod::Lightness:
			{
				const auto [lo, hi] = std::ranges::minmax({r, g, b});
				// x = lo + hi
				// y = 2
				// floor(y / 2) = 1
				l = static_cast<std::uint8_t>((lo + hi + 1) / 2);
				break;
			}
			}

			px[0] = px[1] = px[2] = l;
		}

		return std::make_shared<const Frame>(std::move(out));
	}

	/**
	 * @brief Returns the number of input frames this processor consumes.
	 *
	 * @return `1`.
	 */
	[[nodiscard]] std::size_t input_count() const noexcept override { return 1; }

	/**
	 * @brief Returns the number of output frames this processor produces.
	 *
	 * @return `1`.
	 */
	[[nodiscard]] std::size_t output_count() const noexcept override { return 1; }

	/**
	 * @brief Returns the display label for this processor.
	 *
	 * @return `"Grayscale"`.
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Grayscale"; }

protected:
	/**
	 * @brief Algorithm used to compute the per-pixel intensity.
	 *
	 * @details
	 * Defaults to @ref GrayscaleMethod::Luminosity. Changed at runtime
	 * by @ref GrayscaleProcessorNode.
	 */
	GrayscaleMethod m_method{GrayscaleMethod::Luminosity};
};

} // namespace yellow_mug
