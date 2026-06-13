/**
 * @file source.ixx
 * @brief Declaration and definition of the @ref yellow_mug::SourceProcessor class.
 */

module;
#include "stb_image.h"

export module yellow_mug.processors:source;

import std;
import yellow_mug.core;
import :processor;

export namespace yellow_mug
{

	/**
	 * @brief Processor that loads an image file from disk and emits it as a @ref Frame.
	 *
	 * @details
	 * SourceProcessor is a processor used to open an image file. It consumes
	 * no input frames and produces exactly one output frame. Once an image has been
	 * loaded successfully with @ref load_file(), every subsequent call to `operator()`
	 * returns the cached `shared_ptr` without re-reading the file, so repeated
	 * pipeline evaluations do not incur additional file I/O.
	 *
	 * The tunable state — the file-system path — is a `protected` member so that
	 * the paired @ref SourceProcessorNode can populate an editable text field from
	 * it without exposing that state publicly.
	 *
	 * @see SourceProcessorNode
	 */
	class SourceProcessor : public Processor
	{
	public:
		/**
		 * @brief Default constructor.
		 */
		SourceProcessor() = default;

		/**
		 * @brief Returns the cached frame, or `nullptr` if no image has
		 * been loaded yet.
		 *
		 * @details
		 * The input span is always empty for a source processor and is intentionally
		 * left unnamed. Each call returns the cached `shared_ptr`, sharing
		 * ownership with downstream nodes. Because the frame is `const`,
		 * downstream consumers cannot modify it.
		 *
		 * @return The loaded @ref Frame, or `nullptr` if @ref m_loaded was
		 * not populated using @ref load_file().
		 */
		[[nodiscard]] std::shared_ptr<const Frame> operator()(
			std::span<const std::shared_ptr<const Frame>>) override
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
		 * Decodes the file at @p path to RGBA8 format and stores it in @ref m_loaded.
		 * Additionally, @p path is recorded in @ref m_path; subsequent calls to
		 * @ref operator()() return the newly cached frame.
		 * On failure the previously cached frame, if any, is left unchanged.
		 *
		 * @param path Filesystem path to the image file to decode.
		 */
		void load_file(const std::filesystem::path& path)
		{
			clear_error();
			int width{0};
			int height{0};
			int channels_in_file{0};

			std::uint8_t* const raw = stbi_load(
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
			const auto byte_count = extent.width * extent.height * 4;

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
		 * Accessible to @ref SourceProcessorNode via private inheritance so the
		 * node can distinguish the unloaded state without an explicit accessor.
		 */
		std::shared_ptr<const Frame> m_loaded;

		/**
		 * @brief Filesystem path supplied to the most recent successful
		 * @ref load_file() call.
		 *
		 * @details
		 * Accessible to @ref SourceProcessorNode so it can pre-populate the
		 * editable path text field with the last known path.
		 */
		std::filesystem::path m_path;
	};

} // namespace yellow_mug
