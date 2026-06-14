/**
 * @file crop.ixx
 * @brief Declares the CropProcessor class.
 */
export module yellow_mug.processors:crop;
import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Processor that crops pixels from the edges of a frame.
 *
 * @details
 * Calculates new frame dimensions by subtracting specified top, left, 
 * right, and bottom margins. Returns nullptr and sets an error if the 
 * margins exceed the frame dimensions.
 */
class CropProcessor : public Processor
{
public:
	/**
	 * @brief Crops the input frame.
	 *
	 * @param inputs A span containing exactly one shared pointer to the input frame.
	 * @return A new shared pointer containing the cropped frame, or nullptr on invalid margins.
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

		const auto w = input.dimensions().width;
		const auto h = input.dimensions().height;

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

		Frame out(Extent2D{new_w, new_h});
		
		auto in_bytes = input.bytes();
		auto out_bytes = out.bytes();

		for (auto y{0uz}; y < new_h; ++y)
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

	[[nodiscard]] std::size_t input_count() const noexcept override { return 1; }
	[[nodiscard]] std::size_t output_count() const noexcept override { return 1; }
	[[nodiscard]] std::string_view label() const noexcept override { return "Crop"; }

protected:
	int m_top{0};
	int m_left{0};
	int m_right{0};
	int m_bottom{0};
};

}
