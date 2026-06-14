/**
 * @file grayscale.ixx
 * @brief Declares the GrayscaleProcessor class.
 */
export module yellow_mug.processors:grayscale;
import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Methods for calculating grayscale intensity.
 */
enum class GrayscaleMethod
{
	Luminosity, ///< 0.299 R + 0.587 G + 0.114 B
	Average,    ///< (R + G + B) / 3
	Lightness   ///< (max(R,G,B) + min(R,G,B)) / 2
};

/**
 * @brief Converts an image to grayscale using various configurable methods.
 *
 * @details
 * Calculates the intensity of each pixel using one of three methods: Luminosity,
 * Average, or Lightness. The alpha channel is preserved. Implemented using 
 * integer arithmetic for performance to avoid floating point math.
 */
class GrayscaleProcessor : public Processor
{
public:
	/**
	 * @brief Converts the input frame to grayscale.
	 *
	 * @param inputs A span containing exactly one shared pointer to the input frame.
	 * @return A new shared pointer containing the grayscaled frame, or nullptr if input is missing.
	 */
	[[nodiscard]] std::shared_ptr<const Frame> operator()(
		std::span<const std::shared_ptr<const Frame>> inputs) override
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

		Frame out = input.clone();

		for (auto px : out.pixel_view())
		{
			const std::uint32_t r = px[0];
			const std::uint32_t g = px[1];
			const std::uint32_t b = px[2];
			
			std::uint8_t l = 0;
			switch (m_method)
			{
			case GrayscaleMethod::Luminosity:
				l = static_cast<std::uint8_t>((r * 299 + g * 587 + b * 114) / 1000);
				break;
			case GrayscaleMethod::Average:
				l = static_cast<std::uint8_t>((r + g + b) / 3);
				break;
			case GrayscaleMethod::Lightness:
				l = static_cast<std::uint8_t>((std::max({r, g, b}) + std::min({r, g, b})) / 2);
				break;
			}

			px[0] = px[1] = px[2] = l;
		}

		return std::make_shared<const Frame>(std::move(out));
	}

	/**
	 * @brief Gets the number of inputs required.
	 * @return Always 1.
	 */
	[[nodiscard]] std::size_t input_count() const noexcept override { return 1; }

	/**
	 * @brief Gets the number of outputs produced.
	 * @return Always 1.
	 */
	[[nodiscard]] std::size_t output_count() const noexcept override { return 1; }

	/**
	 * @brief Gets the display label for the processor.
	 * @return "Grayscale".
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Grayscale"; }

protected:
	/// @brief The method used to calculate grayscale intensity.
	GrayscaleMethod m_method{GrayscaleMethod::Luminosity};
};

}
