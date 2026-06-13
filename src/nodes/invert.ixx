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
 * @brief Node wrapping an @ref InvertProcessor in the ImNodeFlow graph.
 *
 * @details
 * InvertProcessorNode inherits privately from @ref InvertProcessor (for its
 * pin layout and @ref Processor::operator()() logic) and publicly from
 * @ref ProcessorNode (for the ImFlow integration).
 *
 * Presents checkboxes to select which channels are inverted.
 */
class InvertProcessorNode : private InvertProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	InvertProcessorNode() : ProcessorNode{static_cast<InvertProcessor&>(*this)}
	{
		setTitle(std::string{this->label()});
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
		ImGui::Checkbox("Red", &m_invert_red);
		ImGui::SameLine();
		ImGui::Checkbox("Green", &m_invert_green);
		ImGui::SameLine();
		ImGui::Checkbox("Blue", &m_invert_blue);
		ImGui::SameLine();
		ImGui::Checkbox("Alpha", &m_invert_alpha);
		draw_error();
	}
};

} // namespace yellow_mug
