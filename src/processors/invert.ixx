/**
 * @file invert.ixx
 * @brief Declaration and definition of the @ref yellow_mug::InvertProcessor class.
 */

export module yellow_mug.processors:invert;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Processor that inverts selectable channels of a frame.
 *
 * @details
 * Applies a bitwise complement (`^= 0xFF`) to each enabled channel of every
 * pixel in the input frame. The red, green, blue, and alpha channels can be
 * toggled independently via the protected boolean members.
 *
 * @see InvertProcessorNode
 */
class InvertProcessor : public Processor
{
public:
	/// @brief Default constructor.
	InvertProcessor() = default;

	/**
	 * @brief Inverts the selected channels of the first input frame.
	 *
	 * @details
	 * Clones @p inputs `[0]` and applies `^= 0xFF` to each channel byte
	 * whose corresponding flag is `true`.
	 *
	 * @param inputs Span containing one element; the frame to invert.
	 * @return The inverted @ref Frame, or `nullptr` if @p inputs `[0]`
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

		auto out = inputs[0]->clone();

		for (auto&& px : out.pixel_view())
		{
			if (m_invert_r) px[0] ^= 0xFF;
			if (m_invert_g) px[1] ^= 0xFF;
			if (m_invert_b) px[2] ^= 0xFF;
			if (m_invert_a) px[3] ^= 0xFF;
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
	 * @return `"Invert"`.
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Invert"; }

protected:
	/// @brief Whether to invert the red channel.
	bool m_invert_r{true};

	/// @brief Whether to invert the green channel.
	bool m_invert_g{true};

	/// @brief Whether to invert the blue channel.
	bool m_invert_b{true};

	/// @brief Whether to invert the alpha channel.
	bool m_invert_a{false};
};

} // namespace yellow_mug
