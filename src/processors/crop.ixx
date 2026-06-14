/**
 * @file crop.ixx
 * @brief Declaration and definition of the @ref yellow_mug::CropProcessor class.
 */

export module yellow_mug.processors:crop;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Processor that trims a rectangular border from a frame.
 *
 * @details
 * Reduces the frame by removing a configurable number of pixels
 * from each edge. All four margins are clamped to zero and validated
 * against the input dimensions before any allocation is performed.
 *
 * @see CropProcessorNode
 */
class CropProcessor : public Processor
{
public:
	/**
	 * @brief Crops the input frame by the configured margins.
	 *
	 * @details
	 * Subtracts @ref m_top, @ref m_left, @ref m_right, and @ref m_bottom
	 * from the input dimensions to determine the output size, then copies
	 * each row of the interior region with `std::copy_n`. Returns `nullptr`
	 * and sets an error when any input is absent, the resulting area is zero,
	 * or the combined margins exceed the frame dimensions.
	 *
	 * @param inputs Span containing one element; the frame to crop.
	 * @return The cropped @ref Frame, or `nullptr` on failure.
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

		const auto& input = *inputs[0];
		if (input.dimensions().empty())
			return std::make_shared<const Frame>(input.clone());

		const auto [w, h] = input.dimensions();

		const auto left = static_cast<std::size_t>(std::max(0, m_left));
		const auto right = static_cast<std::size_t>(std::max(0, m_right));
		const auto top = static_cast<std::size_t>(std::max(0, m_top));
		const auto bottom = static_cast<std::size_t>(std::max(0, m_bottom));

		if (left + right >= w || top + bottom >= h)
		{
			set_error("Crop margins exceed image dimensions.");
			return nullptr;
		}

		const auto new_w = w - (left + right);
		const auto new_h = h - (top + bottom);

		Frame out{{new_w, new_h}};

		const auto in_bytes  = input.bytes();
		auto out_bytes = out.bytes();

		for (const auto y : std::views::iota(0uz, new_h))
		{
			const auto in_y = y + top;
			const auto out_row_start = y * new_w * 4;
			const auto in_row_start = (in_y * w + left) * 4;
			const auto row_bytes = new_w * 4;

			std::copy_n(
				in_bytes.begin() + in_row_start,
				row_bytes,
				out_bytes.begin() + out_row_start
			);
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
	 * @return `"Crop"`.
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Crop"; }

protected:
	/// @brief Pixels to remove from the top edge. Clamped to zero.
	int m_top{0};

	/// @brief Pixels to remove from the left edge. Clamped to zero.
	int m_left{0};

	/// @brief Pixels to remove from the right edge. Clamped to zero.
	int m_right{0};

	/// @brief Pixels to remove from the bottom edge. Clamped to zero.
	int m_bottom{0};
};

} // namespace yellow_mug
