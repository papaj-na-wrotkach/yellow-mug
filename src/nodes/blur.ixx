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
 * @brief Node wrapping a @ref BlurProcessor in the ImNodeFlow graph.
 *
 * @details
 * Inherits privately from @ref BlurProcessor and publicly from
 * @ref ProcessorNode. @ref draw() exposes a slider that controls the
 * blur radius.
 */
class BlurProcessorNode : private BlurProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	BlurProcessorNode() : ProcessorNode{static_cast<BlurProcessor&>(*this)}
	{
		setTitle(std::string{this->label()});
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
