/**
 * @file crop.ixx
 * @brief Declares the CropProcessorNode class.
 */
module;
#include <imgui.h>
export module yellow_mug.nodes:crop;
import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the Crop processor.
 *
 * @details
 * Presents four integer input fields arranged in a cross layout corresponding 
 * to Top, Left, Right, and Bottom crop margins.
 */
class CropProcessorNode : private CropProcessor, public ProcessorNode
{
public:
	CropProcessorNode() : ProcessorNode(static_cast<CropProcessor&>(*this))
	{
		setTitle("Crop");
	}

	void draw() override
	{
		const float item_width = 80.0f;
		const float spacing = ImGui::GetStyle().ItemSpacing.x;

		ImGui::PushItemWidth(item_width);
		
		// Row 1: Top
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + item_width + spacing);
		if (ImGui::InputInt("##top", &m_top)) m_top = std::max(0, m_top);
		
		// Row 2: Left, Center text, Right
		if (ImGui::InputInt("##left", &m_left)) m_left = std::max(0, m_left);
		ImGui::SameLine();
		
		// A dummy element to perfectly space the center based on the item width
		ImGui::Dummy(ImVec2(item_width, 0.0f));
		ImGui::SameLine();
		
		if (ImGui::InputInt("##right", &m_right)) m_right = std::max(0, m_right);
		
		// Row 3: Bottom
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + item_width + spacing);
		if (ImGui::InputInt("##bottom", &m_bottom)) m_bottom = std::max(0, m_bottom);
		
		ImGui::PopItemWidth();

		draw_error();
	}
};

}
