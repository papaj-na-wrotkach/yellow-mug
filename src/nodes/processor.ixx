/**
 * @file processor.ixx
 * @brief Declaration and definition of the @ref yellow_mug::ProcessorNode class.
 */

module;
#include <imgui.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:processor;

import std;
import yellow_mug.core;
import yellow_mug.processors;

export namespace yellow_mug
{

/**
 * @brief Abstract base class binding a @ref Processor to an ImNodeFlow graph node.
 *
 * @details
 * ProcessorNode is the integration layer between the pure processing logic
 * exposed through the @ref Processor interface and the
 * @ref ImFlow::BaseNode infrastructure provided by ImNodeFlow.
 *
 * On construction, ProcessorNode creates one input pin of type
 * `std::shared_ptr<const Frame>` for every slot reported by
 * @ref Processor::input_count(), and, if @ref Processor::output_count()
 * returns `1`, a single output pin whose behaviour pulls all input pin
 * values, forwards them to @ref Processor::operator()(), and returns the
 * result.
 *
 * Every concrete `XxxProcessorNode` is expected to inherit privately from
 * the matching `XxxProcessor` and publicly from ProcessorNode, listing the
 * processor base first in the base-specifier list so that it is fully
 * constructed - vtable included - before the ProcessorNode constructor
 * runs and calls @ref Processor::input_count() and
 * @ref Processor::output_count() through @p processor.
 *
 * @see Processor
 */
class ProcessorNode : public ImFlow::BaseNode
{
public:
	/**
	 * @brief Constructs the node and sets up its pins from @p processor.
	 *
	 * @details
	 * Adds `processor.input_count()` input pins, with integer UIDs
	 * `0` to `input_count() - 1`, each of type `std::shared_ptr<const Frame>`
	 * defaulting to `nullptr`.
	 *
	 * If `processor.output_count()` equals `1`, additionally adds a
	 * single output pin with UID `0` of the same type, whose behaviour
	 * gathers the current values of all input pins into a
	 * `std::vector<std::shared_ptr<const Frame>>`
	 * and forwards it to `processor.operator()()`.
	 *
	 * @param processor Reference to the @ref Processor implementation
	 * that defines this node's pin layout and evaluation logic. The
	 * referenced object must outlive this ProcessorNode; in practice this
	 * is guaranteed by the private-inheritance pattern described in the
	 * class documentation.
	 */
	explicit ProcessorNode(Processor& processor);

	/**
	 * @brief Draws the node's custom content.
	 *
	 * @details
	 * Each concrete node implements its own controls - editable text
	 * fields, sliders, buttons, and so on - according to the parameters
	 * exposed by its paired @ref Processor.
	 */
	void draw() override = 0;

protected:
	/**
	 * @brief Reference to the @ref Processor implementing this node's logic.
	 *
	 * @details
	 * Used polymorphically to query @ref Processor::input_count(),
	 * @ref Processor::output_count(), @ref Processor::label() and to
	 * invoke @ref Processor::operator()() from the output pin's
	 * behaviour. Concrete nodes additionally inherit privately from the
	 * matching `XxxProcessor` to access its tunable parameters directly.
	 */
	Processor& m_processor;

	/**
	 * @brief Helper to display the processor's last error message.
	 *
	 * @details
	 * Checks if the underlying @ref Processor has set an error message
	 * and if so, displays it in red text within the node. Should be
	 * called by derived classes in their @ref draw() implementation.
	 */
	void draw_error() const
	{
		auto err = m_processor.last_error();
		if (!err.empty())
		{
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", err.data());
		}
	}
};

ProcessorNode::ProcessorNode(Processor& processor)
	: m_processor(processor)
{
	for (auto i{0uz}; i < m_processor.input_count(); ++i)
	{
		addIN_uid<std::shared_ptr<const Frame>>(i, std::to_string(i), nullptr, ImFlow::ConnectionFilter::None());
	}

	if (m_processor.output_count() == 1)
	{
		addOUT_uid<std::shared_ptr<const Frame>>(0, "out")
			->behaviour([this]() -> std::shared_ptr<const Frame>
			{
				std::vector<std::shared_ptr<const Frame>> inputs =
					std::views::iota(0uz, m_processor.input_count())
					| std::views::transform([this](std::size_t i)
					{
						return getInVal<std::shared_ptr<const Frame>>(i);
					})
					| std::ranges::to<std::vector>();

				return m_processor(inputs);
			});
	}
}

} // namespace yellow_mug
