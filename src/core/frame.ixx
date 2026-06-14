/**
 * @file frame.ixx
 * @brief Declaration and definition of the @ref yellow_mug::Extent2D struct
 *        and the @ref yellow_mug::Frame class.
 */
export module yellow_mug.core:frame;
import std;

export namespace yellow_mug
{

/**
 * @brief Stores image dimensions.
 */
struct Extent2D
{
	/// @brief Width, in pixels.
	std::size_t width{0uz};

	/// @brief Height, in pixels.
	std::size_t height{0uz};

	/**
	 * @brief Default member-wise equality operator.
	 *
	 * @param other The other extent to compare against.
	 * @return True if both width and height are equal, false otherwise.
	 */
	[[nodiscard]] bool operator==(const Extent2D& other) const noexcept = default;

	/**
	 * @brief Returns the total area of the extent in pixels.
	 *
	 * @return The product of width and height.
	 */
	[[nodiscard]] std::size_t area() const noexcept
	{
		return width * height;
	}

	/**
	 * @brief Checks if either dimension is zero.
	 *
	 * @return True if width or height is zero, false otherwise.
	 */
	[[nodiscard]] bool empty() const noexcept
	{
		return !width || !height;
	}

	/**
	 * @brief Calculates the aspect ratio of the extent.
	 *
	 * @return The ratio of width to height, or 0.0 if height is zero.
	 */
	[[nodiscard]] double aspect_ratio() const noexcept
	{
		return !height ? 0.0 : static_cast<double>(width) / height;
	}

	/**
	 * @brief Component-wise addition.
	 *
	 * @param other The extent to add.
	 * @return A new extent with summed dimensions.
	 */
	[[nodiscard]] Extent2D operator+(const Extent2D& other) const noexcept
	{
		return {width + other.width, height + other.height};
	}

	/**
	 * @brief Component-wise subtraction.
	 *
	 * @param other The extent to subtract.
	 * @return A new extent with subtracted dimensions.
	 */
	[[nodiscard]] Extent2D operator-(const Extent2D& other) const noexcept
	{
		return {width - other.width, height - other.height};
	}

	/**
	 * @brief Component-wise addition compound assignment.
	 *
	 * @param other The extent to add.
	 * @return A reference to this extent.
	 */
	Extent2D& operator+=(const Extent2D& other) & noexcept
	{
		width += other.width;
		height += other.height;
		return *this;
	}

	/**
	 * @brief Component-wise subtraction compound assignment.
	 *
	 * @param other The extent to subtract.
	 * @return A reference to this extent.
	 */
	Extent2D& operator-=(const Extent2D& other) & noexcept
	{
		width -= other.width;
		height -= other.height;
		return *this;
	}

	/**
	 * @brief Stream output operator.
	 *
	 * @param os Output stream to write to.
	 * @param extent Extent to format.
	 * @return Reference to the output stream.
	 */
	friend std::ostream& operator<<(std::ostream& os, const Extent2D& extent)
	{
		return os << extent.width << "x" << extent.height;
	}
};

/**
 * @brief A rectangular RGBA8 image buffer.
 *
 * @details
 * A `Frame` owns a contiguous buffer of `width * height * 4` bytes,
 * storing pixels in row-major order with four 8-bit channels per pixel
 * (red, green, blue, alpha).
 *
 * The type is move-only. Copying is intentionally disabled to make
 * duplication costs visible at call sites; use @ref clone()
 * to get an explicit deep copy when one is required.
 */
class Frame
{
public:
	/**
	 * @brief Constructs an empty frame with zero width and height.
	 */
	Frame() = default;

	/**
	 * @brief Constructs a frame with the given dimensions.
	 *
	 * @details
	 * Allocates a pixel buffer of `dimensions.area() * 4`
	 * bytes, zero-initialized.
	 *
	 * @param dimensions Width and height of the frame, in pixels.
	 */
	explicit Frame(const Extent2D dimensions) : m_extent{dimensions}
	{
	}

	/**
	 * @brief Constructs a frame from already-decoded pixel data.
	 *
	 * @details
	 * Copies @p data into the frame's internal pixel buffer. This is the
	 * intended constructor for populating a `Frame` from the output of an
	 * external image decoder (e.g., `stb_image`), without coupling `Frame`
	 * to that library's types.
	 *
	 * @param dimensions Width and height of the frame, in pixels.
	 * @param data Decoded RGBA8 pixel data, row-major, of size
	 *             `dimensions.area() * 4`.
	 *
	 * @pre `data.size() == dimensions.area() * 4`.
	 */
	Frame(const Extent2D dimensions, const std::span<const std::uint8_t> data) :
		m_extent{dimensions},
		m_pixels{data.begin(), data.end()}
	{
	}

	/// @brief Copy construction is disabled; use @ref clone() for explicit deep copies.
	Frame(const Frame&) = delete;

	/// @brief Copy assignment is disabled; use @ref clone() for explicit deep copies.
	Frame& operator=(const Frame&) = delete;

	/// @brief Move-constructs a frame, transferring ownership of the pixel buffer.
	Frame(Frame&&) = default;

	/// @brief Move-assigns a frame, transferring ownership of the pixel buffer.
	Frame& operator=(Frame&&) = default;

	/**
	 * @brief Creates an explicit deep copy of this frame.
	 *
	 * @return A new `Frame` with the same dimensions and pixel data.
	 */
	[[nodiscard]] Frame clone() const
	{
		return {m_extent, m_pixels};
	}

	/**
	 * @brief Returns the dimensions of the frame.
	 *
	 * @return A read-only reference to the frame's width and height.
	 */
	[[nodiscard]] const Extent2D& dimensions() const noexcept
	{
		return m_extent;
	}

	/**
	 * @brief Returns a view over the pixel data, grouped into 4-byte pixels.
	 *
	 * @details
	 * The returned range yields one 4-byte subrange per pixel, in
	 * row-major order, each containing the red, green, blue, and alpha
	 * channels in that order. Elements of the subranges reference the
	 * underlying buffer and can be written through.
	 *
	 * @return A view of `width * height` mutable 4-byte pixel subranges.
	 */
	[[nodiscard]] auto pixel_view() noexcept
	{
		return m_pixels | std::views::chunk(4);
	}

	/**
	 * @brief Returns a read-only view over the pixel data, grouped into 4-byte pixels.
	 *
	 * @return A view of `width * height` read-only 4-byte pixel subranges.
	 *
	 * @sa pixel_view()
	 */
	[[nodiscard]] auto pixel_view() const noexcept
	{
		return m_pixels | std::views::chunk(4);
	}

	/**
	 * @brief Returns the raw pixel buffer as a flat byte span.
	 *
	 * @details
	 * Intended for interoperability with external libraries that operate
	 * on flat byte buffers, such as image decoders or GPU texture uploads.
	 *
	 * @return A span over the `width * height * 4` pixel bytes.
	 */
	[[nodiscard]] std::span<std::uint8_t> bytes() noexcept
	{
		return m_pixels;
	}

	/**
	 * @brief Returns the raw pixel buffer as a flat read-only byte span.
	 *
	 * @return A read-only span over the `width * height * 4` pixel bytes.
	 *
	 * @sa bytes()
	 */
	[[nodiscard]] std::span<const std::uint8_t> bytes() const noexcept
	{
		return m_pixels;
	}

	/**
	 * @brief Reads raw pixel data for a frame from a binary input stream.
	 *
	 * @details
	 * Reads exactly `frame.dimensions().area() * 4`
	 * bytes from @p in into @p frame's pixel buffer. The frame's dimensions
	 * must already be set by the caller before this operator is used; the
	 * stream is expected to contain only raw, uncompressed RGBA8 pixel data
	 * with no header. Compressed image formats are not supported here and
	 * must be decoded separately before being placed into a `Frame`.
	 *
	 * @param in Binary input stream to read from.
	 * @param frame Frame whose pixel buffer is filled with the data read.
	 * @return Reference to @p in, for chaining.
	 */
	friend std::istream& operator>>(std::istream& in, Frame& frame)
	{
		in.read(
			reinterpret_cast<char*>(frame.m_pixels.data()),
			static_cast<std::streamsize>(frame.m_pixels.size())
		);
		return in;
	}

private:
	/// @brief Width and height of the frame, in pixels.
	Extent2D m_extent{0, 0};

	/**
	 * @brief Pixel storage, `area() * 4` bytes in row-major RGBA8 order.
	 */
	std::vector<std::uint8_t> m_pixels = std::vector<std::uint8_t>(m_extent.area() * 4);
};

} // namespace yellow_mug
