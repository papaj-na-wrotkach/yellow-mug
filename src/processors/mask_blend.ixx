/**
 * @file mask_blend.ixx
 * @brief Declaration and definition of the @ref yellow_mug::MaskBlendProcessor class.
 */

export module yellow_mug.processors:mask_blend;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Selects which colour channel of the mask frame drives the blend.
 *
 * @see MaskBlendProcessor::m_mask_channel
 */
enum class MaskChannel : std::size_t
{
	Red = 0,
	Green = 1,
	Blue = 2,
	Alpha = 3,
};

/**
 * @brief Processor that linearly blends two frames according to a mask.
 *
 * @details
 * Consumes three input frames — source, target, and mask — and produces one
 * output frame. Each output pixel is a linear interpolation between
 * the corresponding source and target pixels, with one selectable channel
 * of the mask pixel (see @ref m_mask_channel) used as the normalised
 * blend weight.
 *
 * @see MaskBlendProcessorNode
 */
class MaskBlendProcessor : public Processor
{
public:
	/**
	 * @brief Default constructor.
	 */
	MaskBlendProcessor() = default;

	/**
	 * @brief Blends the source and target frames according to the mask frame.
	 *
	 * @details
	 * Input slot layout:
	 * - Slot 0 — source frame (fully present where the mask is black, i.e. @em alpha = 0).
	 * - Slot 1 — target frame (fully present where the mask is white, i.e. @em alpha = 1).
	 * - Slot 2 — mask frame; the channel selected by @ref m_mask_channel is
	 *   read as the blend weight.
	 *
	 * A freshly allocated output @ref Frame with the same @ref Extent2D as the
	 * inputs is produced.
	 *
	 * @param inputs Span of three `shared_ptr` in source, target,
	 * mask order (`nullptr` when absent).
	 * @return The blended output @ref Frame, or `nullptr` if any input slot
	 * is empty or if the frames differ in dimensions.
	 */
	[[nodiscard]] std::shared_ptr<const Frame> operator()(
		std::span<const std::shared_ptr<const Frame>> inputs) override
	{
		clear_error();
		if (!(inputs[0] && inputs[1] && inputs[2]))
		{
			set_error("Missing one or more input frames.");
			return nullptr;
		}

		const Frame& src = *inputs[0];
		const Frame& tgt = *inputs[1];
		const Frame& msk = *inputs[2];

		const Extent2D& extent = src.dimensions();

		if (tgt.dimensions() != extent || msk.dimensions() != extent)
		{
			set_error("Input frames have mismatched dimensions.");
			return nullptr;
		}

		Frame out(extent);

		for (auto&& [s, t, m, o] : std::views::zip(
			src.pixel_view(),
			tgt.pixel_view(),
			msk.pixel_view(),
			out.pixel_view()))
		{
			const float alpha = static_cast<float>(
				m[static_cast<std::size_t>(m_mask_channel)]) / 255.0f;

			// mask-blend RGB channels
			for (auto c{0uz}; c < 3; ++c)
			{
				o[c] = static_cast<std::uint8_t>(std::lerp(
					static_cast<float>(s[c]),
					static_cast<float>(t[c]),
					alpha
				));
			}

			// copy alpha channel
			o[3] = s[3];
		}

		return std::make_shared<const Frame>(std::move(out));
	}

	/**
	 * @brief Returns the number of input frames this processor consumes.
	 *
	 * @return `3`.
	 */
	[[nodiscard]] std::size_t input_count() const noexcept override { return 3; }

	/**
	 * @brief Returns the number of output frames this processor produces.
	 *
	 * @return `1`.
	 */
	[[nodiscard]] std::size_t output_count() const noexcept override { return 1; }

	/**
	 * @brief Returns the display label for this processor.
	 *
	 * @return `"Mask Blend"`.
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Mask Blend"; }

protected:
	/**
	 * @brief Which channel of the mask frame to sample.
	 *
	 * @details
	 * Defaults to @ref MaskChannel::Alpha. Accessible to
	 * @ref MaskBlendProcessorNode via private inheritance so the node
	 * can present radio-button controls.
	 */
	MaskChannel m_mask_channel{MaskChannel::Alpha};
};

} // namespace yellow_mug
