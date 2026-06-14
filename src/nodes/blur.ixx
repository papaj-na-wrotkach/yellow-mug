/**
 * @file blur.ixx
 * @brief Declaration and definition of the @ref yellow_mug::BlurProcessorNode class.
 */

module;
#include <imgui.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:blur;

import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the @ref BlurProcessor.
 *
 * @details
 * Presents a radius slider that controls @ref BlurProcessor::m_radius,
 * clamped to `[0, 64]`.
 *
 * @see BlurProcessor
 */
class BlurProcessorNode final : BlurProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	BlurProcessorNode() : ProcessorNode{static_cast<BlurProcessor&>(*this)}
	{
		setTitle(std::string{this->BlurProcessor::label()});
	}

	/**
	 * @brief Draws the blur radius slider.
	 *
	 * @details
	 * Presents an `ImGui::SliderInt` clamped to `[0, 64]` that controls
	 * @ref BlurProcessor::m_radius.
	 */
	void draw() override
	{
		ImGui::SetNextItemWidth(150.f);
		ImGui::SliderInt("Radius", &m_radius, 0, 64);
		draw_error();
	}
};

} // namespace yellow_mug
