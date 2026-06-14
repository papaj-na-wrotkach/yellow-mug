/**
 * @file source.ixx
 * @brief Declaration and definition of the @ref yellow_mug::SourceProcessorNode class.
 */

module;
#include <imgui.h>
#include <imgui_stdlib.h>
#include <ImNodeFlow.h>

export module yellow_mug.nodes:source;

import std;
import yellow_mug.core;
import yellow_mug.processors;
import :processor;

export namespace yellow_mug
{

/**
 * @brief Node integration for the @ref SourceProcessor.
 *
 * @details
 * Presents an editable text field for the image path and a "Load" button
 * that triggers @ref SourceProcessor::load_file().
 *
 * @see SourceProcessor
 */
class SourceProcessorNode final : SourceProcessor, public ProcessorNode
{
public:
	/**
	 * @brief Constructs the node and sets its title from @ref Processor::label().
	 */
	SourceProcessorNode() : ProcessorNode{static_cast<SourceProcessor&>(*this)}
	{
		setTitle(std::string{this->SourceProcessor::label()});
	}

	/**
	 * @brief Draws the path input field and "Load" button.
	 *
	 * @details
	 * Displays an editable text field bound to @ref m_path_buffer. When
	 * the "Load" button is pressed, @ref m_path_buffer is forwarded to
	 * @ref SourceProcessor::load_file(); if loading fails, an error
	 * message is shown below the button.
	 */
	void draw() override;

private:
	/**
	 * @brief Editable text buffer backing the path input field.
	 *
	 * @details
	 * Holds the user-edited path as plain text, independent of
	 * @ref SourceProcessor::m_path, which is only updated on a successful
	 * @ref SourceProcessor::load_file() call.
	 */
	std::string m_path_buffer;
};

void SourceProcessorNode::draw()
{
	ImGui::SetNextItemWidth(200.f);
	ImGui::InputText("Path", &m_path_buffer);

	if (ImGui::Button("Load"))
	{
		load_file(m_path_buffer);
	}

	draw_error();
}

} // namespace yellow_mug
