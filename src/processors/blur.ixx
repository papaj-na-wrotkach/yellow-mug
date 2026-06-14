/**
 * @file blur.ixx
 * @brief Declaration and definition of the @ref yellow_mug::BlurProcessor class.
 */

export module yellow_mug.processors:blur;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Processor that applies a separable box blur to a frame.
 *
 * @details
 * Performs a two-pass separable box blur on a single input frame.
 * The horizontal pass averages pixels within a `(2 * m_radius + 1)`-wide
 * window along each row, then the vertical pass does the same along each
 * column of the intermediate result. Edge pixels use a reduced window
 * clamped to the frame boundary.
 *
 * @see BlurProcessorNode
 */
class BlurProcessor : public Processor
{
public:
	/**
	 * @brief Default constructor.
	 */
	BlurProcessor() = default;

	/**
	 * @brief Applies a separable box blur to the first input frame.
	 *
	 * @details
	 * The blur operates in premultiplied-alpha space to avoid colour bleeding
	 * from fully transparent pixels. It relies entirely on `std::views` and
	 * exact 32-bit integer arithmetic (no floats). The steps are:
	 * -# Premultiply: scale each RGB channel by `A` into an `uint32_t` buffer.
	 * -# Horizontal pass: blur pixels horizontally.
	 * -# Vertical pass: blur pixels vertically.
	 * -# Unpremultiply: divide the accumulated RGB sums by the blurred alpha.
	 *
	 * A radius of `0` short-circuits to a plain clone.
	 *
	 * @param inputs Span containing one element; the frame to blur.
	 * @return The blurred @ref Frame or a clone of the input frame if
	 *         @ref m_radius is `0` or `nullptr` if `inputs[0]` is `nullptr`.
	 *
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
		const auto [width, height] = input.dimensions();
		if (input.dimensions().empty() || m_radius == 0)
			return std::make_shared<const Frame>(input.clone());

		const auto r = static_cast<std::size_t>(m_radius);
		const auto w_size = 2 * r + 1;
		const auto pixel_count = input.dimensions().area();

		using Pixel = std::array<std::uint32_t, 4>;
		std::vector<Pixel> mid_grid(pixel_count);
		std::vector<Pixel> padded(std::max(width, height) + 2 * r);

		const auto in = input.bytes();

		// 1. Horizontal pass
		for (auto y{0uz}; y < height; ++y)
		{
			// Pad the row into padded and premultiply directly.
			for (auto x{0uz}; x < width; ++x)
			{
				const auto idx = (y * width + x) * 4;
				// Extract the alpha channel and cast it.
				const std::uint32_t a = in[idx + 3];
				// Multiply RGB channels by alpha.
				padded[x + r] = {in[idx] * a, in[idx + 1] * a, in[idx + 2] * a, a};
			}

			// Fill left and right borders
			std::fill_n(padded.begin(), r, padded[r]);
			std::fill_n(padded.begin() + width + r, r, padded[width + r - 1]);

			// Blur the row.
			for (auto x{0uz}; x < width; ++x)
			{
				std::uint32_t sr{0}, sg{0}, sb{0}, sa{0};
				for (auto k{0uz}; k < w_size; ++k)
				{
					const auto& px = padded[x + k];
					sr += px[0]; sg += px[1]; sb += px[2]; sa += px[3];
				}
				mid_grid[y * width + x] = {sr, sg, sb, sa};
			}
		}

		Frame out{input.dimensions()};
		auto out_bytes = out.bytes();
		const auto area = static_cast<std::uint32_t>(w_size * w_size);

		// 2. Vertical pass & Unpremultiply
		for (auto x{0uz}; x < width; ++x)
		{
			// Pad the column into padded
			for (auto y{0uz}; y < height; ++y)
			{
				padded[y + r] = mid_grid[y * width + x];
			}

			// Fill top and bottom borders
			std::fill_n(padded.begin(), r, padded[r]);
			std::fill_n(padded.begin() + height + r, r, padded[height + r - 1]);

			// Blur the column
			for (auto y{0uz}; y < height; ++y)
			{
				std::uint32_t sr{0}, sg{0}, sb{0}, sa{0};
				for (auto k{0uz}; k < w_size; ++k)
				{
					const auto& px = padded[y + k];
					sr += px[0]; sg += px[1]; sb += px[2]; sa += px[3];
				}

				const auto idx = (y * width + x) * 4;
				if (sa > 0)
				{
					out_bytes[idx] = static_cast<std::uint8_t>(sr / sa);
					out_bytes[idx + 1] = static_cast<std::uint8_t>(sg / sa);
					out_bytes[idx + 2] = static_cast<std::uint8_t>(sb / sa);
				}
				else
				{
					out_bytes[idx] = out_bytes[idx + 1] = out_bytes[idx + 2] = 0;
				}
				out_bytes[idx + 3] = static_cast<std::uint8_t>(sa / area);
			}
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
	 * @return `"Blur"`.
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Blur"; }

protected:
	/**
	 * @brief Blur kernel radius.
	 *
	 * @details
	 * The full kernel width is `2 * m_radius + 1`. Clamped to the
	 * range `[0, 64]` by the @ref BlurProcessorNode slider. A value
	 * of `0` disables blurring.
	 */
	int m_radius{1};
};

} // namespace yellow_mug
