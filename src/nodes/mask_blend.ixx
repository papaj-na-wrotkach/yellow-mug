/**
 * @file mask_blend.ixx
 * @brief Declaration and definition of the @ref yellow_mug::MaskBlendProcessorNode class.
 */

module;
#include <imgui.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:mask_blend;

import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the @ref MaskBlendProcessor.
 *
 * @details
 * Presents radio buttons that select which channel of the mask frame drives
 * the per-pixel blend weight (see @ref MaskBlendProcessor::m_mask_channel).
 * The three input pins correspond to the source, target, and mask frames.
 *
 * @see MaskBlendProcessor
 */
class MaskBlendProcessorNode final : MaskBlendProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	MaskBlendProcessorNode() : ProcessorNode{static_cast<MaskBlendProcessor&>(*this)}
	{
		setTitle(std::string{this->MaskBlendProcessor::label()});
	}

	/**
	 * @brief Draws the mask channel radio-button selector.
	 *
	 * @details
	 * Presents four `ImGui::RadioButton` controls ("Red", "Green",
	 * "Blue", "Alpha") that select the @ref MaskBlendProcessor::m_mask_channel
	 * used during the next evaluation.
	 */
	void draw() override
	{
		auto channel = static_cast<int>(m_mask_channel);
		ImGui::TextUnformatted("Mask channel");
		ImGui::RadioButton("Red", &channel, static_cast<int>(MaskChannel::Red));
		ImGui::RadioButton("Green", &channel, static_cast<int>(MaskChannel::Green));
		ImGui::RadioButton("Blue", &channel, static_cast<int>(MaskChannel::Blue));
		ImGui::RadioButton("Alpha", &channel, static_cast<int>(MaskChannel::Alpha));
		m_mask_channel = static_cast<MaskChannel>(channel);
		draw_error();
	}
};

} // namespace yellow_mug
