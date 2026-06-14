/**
 * @file invert.ixx
 * @brief Declaration and definition of the @ref yellow_mug::InvertProcessorNode class.
 */

module;
#include <imgui.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:invert;

import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the @ref InvertProcessor.
 *
 * @details
 * Presents a set of checkboxes that independently toggle inversion of the
 * red, green, blue, and alpha channels.
 *
 * @see InvertProcessor
 */
class InvertProcessorNode final : InvertProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	InvertProcessorNode() : ProcessorNode{static_cast<InvertProcessor&>(*this)}
	{
		setTitle(std::string{this->InvertProcessor::label()});
	}

	/**
	 * @brief Draws the node's channel-selection checkboxes.
	 *
	 * @details
	 * Presents four `ImGui::Checkbox` controls that select which
	 * channels of the frame to invert.
	 */
	void draw() override
	{
		ImGui::Checkbox("Red", &m_invert_r);
		ImGui::Checkbox("Green", &m_invert_g);
		ImGui::Checkbox("Blue", &m_invert_b);
		ImGui::Checkbox("Alpha", &m_invert_a);
		draw_error();
	}
};

} // namespace yellow_mug
