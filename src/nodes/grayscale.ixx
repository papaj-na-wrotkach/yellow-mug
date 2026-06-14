/**
 * @file grayscale.ixx
 * @brief Declaration and definition of the @ref yellow_mug::GrayscaleProcessorNode class.
 */

module;
#include <imgui.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:grayscale;

import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the @ref GrayscaleProcessor.
 *
 * @details
 * Presents a combo box that lets the user select the @ref GrayscaleMethod
 * used to compute the per-pixel intensity.
 *
 * @see GrayscaleProcessor
 */
class GrayscaleProcessorNode final : GrayscaleProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	GrayscaleProcessorNode() : ProcessorNode{static_cast<GrayscaleProcessor&>(*this)}
	{
		setTitle(std::string{this->GrayscaleProcessor::label()});
	}

	/**
	 * @brief Draws the grayscale method combo box.
	 *
	 * @details
	 * Presents an `ImGui::Combo` listing all @ref GrayscaleMethod values
	 * (`Luminosity`, `Average`, `Lightness`), bound to
	 * @ref GrayscaleProcessor::m_method.
	 */
	void draw() override
	{
		auto method = static_cast<int>(m_method);
		constexpr std::array items{
			"Luminosity",
			"Average",
			"Lightness"
		};

		ImGui::SetNextItemWidth(100.0f);
		if (ImGui::Combo("Method", &method, items.data(), 3))
		{
			m_method = static_cast<GrayscaleMethod>(method);
		}
	}
};

} // namespace yellow_mug
