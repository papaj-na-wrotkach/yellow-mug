/**
 * @file crop.ixx
 * @brief Declaration and definition of the @ref yellow_mug::CropProcessorNode class.
 */

module;
#include <imgui.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:crop;

import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the @ref CropProcessor.
 *
 * @details
 * Presents four integer input fields — Top, Left, Right, and Bottom — arranged
 * in a cross layout that mirrors the spatial meaning of each margin.
 *
 * @see CropProcessor
 */
class CropProcessorNode final : CropProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	CropProcessorNode() : ProcessorNode{static_cast<CropProcessor&>(*this)}
	{
		setTitle(std::string{this->CropProcessor::label()});
	}

	/**
	 * @brief Draws the crop margin input fields.
	 *
	 * @details
	 * Presents four `ImGui::InputInt` controls for @ref CropProcessor::m_top,
	 * @ref CropProcessor::m_left, @ref CropProcessor::m_right, and
	 * @ref CropProcessor::m_bottom, arranged in a cross layout. Each value
	 * is clamped to zero on input. @ref ProcessorNode::draw_error() is called
	 * to display any error set during the last evaluation.
	 */
	void draw() override
	{
		constexpr auto item_width = 80.0f;
		const auto spacing = ImGui::GetStyle().ItemSpacing.x;

		ImGui::PushItemWidth(item_width);

		// Row 1: Top
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + item_width + spacing);
		if (ImGui::InputInt("##top", &m_top)) m_top = std::max(0, m_top);

		// Row 2: Left, (space), Right
		if (ImGui::InputInt("##left", &m_left)) m_left = std::max(0, m_left);
		ImGui::SameLine();
		ImGui::Dummy(ImVec2{item_width, 0.0f});
		ImGui::SameLine();
		if (ImGui::InputInt("##right", &m_right)) m_right = std::max(0, m_right);

		// Row 3: Bottom
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + item_width + spacing);
		if (ImGui::InputInt("##bottom", &m_bottom)) m_bottom = std::max(0, m_bottom);

		ImGui::PopItemWidth();

		draw_error();
	}
};

} // namespace yellow_mug
