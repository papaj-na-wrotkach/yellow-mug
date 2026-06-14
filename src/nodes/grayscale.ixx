/**
 * @file grayscale.ixx
 * @brief Declares the GrayscaleProcessorNode class.
 */
module;
#include <imgui.h>
export module yellow_mug.nodes:grayscale;
import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the Grayscale processor.
 *
 * @details
 * Inherits privately from @ref GrayscaleProcessor to construct the
 * processor subsystem before the node, and publicly from @ref ProcessorNode
 * to provide the ImNodeFlow UI elements.
 */
class GrayscaleProcessorNode : private GrayscaleProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and binds it to its internal processor.
	 */
	GrayscaleProcessorNode() : ProcessorNode(static_cast<GrayscaleProcessor&>(*this))
	{
		setTitle("Grayscale");
	}

	/**
	 * @brief Draws the node's UI, including a combo box for selecting the grayscale method.
	 */
	void draw() override
	{
		int method = static_cast<int>(m_method);
		constexpr std::array items = { "Luminosity", "Average", "Lightness" };
		
		ImGui::SetNextItemWidth(100.0f);
		if (ImGui::Combo("Method", &method, items.data(), 3))
		{
			m_method = static_cast<GrayscaleMethod>(method);
		}
	}
};

}
