/**
 * @file source.ixx
 * @brief Declaration and definition of the @ref yellow_mug::SourceProcessor class.
 */

module;
#include <stb_image.h>

export module yellow_mug.processors:source;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Processor that loads an image file from the disk and emits it as a @ref Frame.
 *
 * @details
 * Consumes no input frames and produces exactly one output frame. Once an
 * image has been loaded successfully with @ref load_file(), every subsequent
 * call to `operator()` returns the cached `shared_ptr` without re-reading
 * the file, so repeated pipeline evaluations do not incur additional file I/O.
 *
 * @see SourceProcessorNode
 */
class SourceProcessor : public Processor
{
public:
	/// @brief Default constructor.
	SourceProcessor() = default;

	/**
	 * @brief Returns the cached frame, or `nullptr` if no image has been loaded.
	 *
	 * @details
	 * The @p inputs span is always empty for this processor and is left unnamed.
	 * Each call shares ownership of the cached frame with the caller. Because
	 * the pointed-to @ref Frame is `const`, downstream consumers cannot modify it.
	 *
	 * @return The loaded @ref Frame, or `nullptr` if @ref m_loaded has not been
	 *         populated by a successful @ref load_file() call.
	 */
	[[nodiscard]] std::shared_ptr<const Frame> operator()(
		const std::span<const std::shared_ptr<const Frame>>) override
	{
		clear_error();
		if (!m_loaded)
		{
			set_error("No image loaded.");
		}
		return m_loaded;
	}

	/**
	 * @brief Returns the number of input frames this processor consumes.
	 *
	 * @return `0`.
	 */
	[[nodiscard]] std::size_t input_count() const noexcept override { return 0; }

	/**
	 * @brief Returns the number of output frames this processor produces.
	 *
	 * @return `1`.
	 */
	[[nodiscard]] std::size_t output_count() const noexcept override { return 1; }

	/**
	 * @brief Returns the display label for this processor.
	 *
	 * @return `"Source"`.
	 */
	[[nodiscard]] std::string_view label() const noexcept override { return "Source"; }

	/**
	 * @brief Loads and caches an image from a file.
	 *
	 * @details
	 * Decodes the file at @p path to RGBA8 format using `stb_image` and stores
	 * the result in @ref m_loaded. The @p path is also stored in @ref m_path.
	 * On failure the previously cached frame, if any, is left unchanged, and an
	 * error is set via @ref Processor::set_error().
	 *
	 * @param path Filesystem path to the image file to decode.
	 */
	void load_file(const std::filesystem::path& path)
	{
		clear_error();
		auto width{0};
		auto height{0};
		auto channels_in_file{0};

		auto* const raw = stbi_load(
			path.string().c_str(),
			&width,
			&height,
			&channels_in_file,
			4
		);

		if (raw == nullptr)
		{
			set_error("Failed to load image from file.");
			return;
		}

		const Extent2D extent{
			static_cast<std::size_t>(width),
			static_cast<std::size_t>(height)
		};
		const auto byte_count = extent.area() * 4;

		m_loaded = std::make_shared<const Frame>(extent, std::span<const std::uint8_t>(raw, byte_count));
		m_path = path;

		stbi_image_free(raw);
	}

protected:
	/**
	 * @brief Cached frame from the most recent successful @ref load_file() call.
	 *
	 * @details
	 * Holds `nullptr` until @ref load_file() succeeds at least once.
	 */
	std::shared_ptr<const Frame> m_loaded{nullptr};

	/**
	 * @brief Filesystem path of the most recent successful @ref load_file() call.
	 *
	 * @details
	 * Accessible to @ref SourceProcessorNode so it can pre-populate the
	 * editable path text field with the last known value.
	 */
	std::filesystem::path m_path{};
};

} // namespace yellow_mug
