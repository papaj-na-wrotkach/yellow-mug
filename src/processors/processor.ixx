/**
 * @file processor.ixx
 * @brief Declaration of the abstract Processor interface.
 */

export module yellow_mug.processors:processor;

import std;
import yellow_mug.core;

export namespace yellow_mug
{

/**
 * @brief Abstract interface for a single image processing operation.
 *
 * @details
 * A Processor represents the pure logic of one node of the image processing
 * pipeline, independent of any user interface or graph library. It
 * declares how many input frames it consumes, how many output frames it
 * produces, the operation that transforms the inputs into an output, and a
 * human-readable label identifying it.
 *
 * Concrete processors derive publicly from this class and implement
 * `operator()`, `input_count()`, `output_count()` and `label()` according to
 * their specific behaviour. Tunable parameters of a concrete processor
 * should be declared as protected members, so that the corresponding
 * `ProcessorNode`, which inherits privately from the processor, can access
 * and expose them through the graphical interface.
 *
 * @see ProcessorNode
 */
class Processor
{
public:
	/**
	 * @brief Virtual destructor.
	 *
	 * @details
	 * Ensures that derived processors are destroyed correctly when accessed
	 * through a reference or pointer to Processor.
	 */
	virtual ~Processor() = default;

	/**
	 * @brief Returns the last error message set by this processor.
	 *
	 * @return A string view to the error message, empty if no error.
	 */
	[[nodiscard]] std::string_view last_error() const noexcept { return m_last_error; }

	/**
	 * @brief Executes the processing operation.
	 *
	 * @details
	 * Each element of @p inputs corresponds to one of this processor's input
	 * slots, in the same order as declared by @ref input_count(). An element
	 * equal to `nullptr` means that the corresponding input is either not
	 * connected or produced no frame.
	 *
	 * Implementations must not modify the pointed-to input frames in place;
	 * if a frame needs to be altered, it must first be duplicated with
	 * @ref Frame::clone().
	 *
	 * @param inputs View over the input frames, with one `shared_ptr`
	 * per input slot (`nullptr` when absent).
	 *
	 * @return The resulting frame, or `nullptr` if no output frame can be
	 * produced for the given inputs. `nullptr` is the only mechanism used
	 * to signal that no result is available.
	 */
	[[nodiscard]] virtual std::shared_ptr<const Frame> operator()(
		std::span<const std::shared_ptr<const Frame>> inputs) = 0;

	/**
	 * @brief Returns the number of input frames this processor consumes.
	 *
	 * @return Number of input slots.
	 */
	[[nodiscard]] virtual std::size_t input_count() const noexcept = 0;

	/**
	 * @brief Returns the number of output frames this processor produces.
	 *
	 * @return Number of output slots, either 0 or 1.
	 */
	[[nodiscard]] virtual std::size_t output_count() const noexcept = 0;

	/**
	 * @brief Returns a human-readable name for this processor.
	 *
	 * @details
	 * The returned name is used as the display title of the corresponding
	 * node in the graphical editor.
	 *
	 * @return Short descriptive label of the processor.
	 */
	[[nodiscard]] virtual std::string_view label() const noexcept = 0;

protected:
	/**
	 * @brief Sets the error message.
	 *
	 * @param msg The error message to set.
	 */
	void set_error(std::string msg) { m_last_error = std::move(msg); }

	/**
	 * @brief Clears the current error message.
	 */
	void clear_error() noexcept { m_last_error.clear(); }

private:
	/**
	 * @brief Holds the last error message produced by this processor.
	 */
	std::string m_last_error;
};

}
