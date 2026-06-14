/**
 * @file target.ixx
 * @brief Declaration and definition of the @ref yellow_mug::TargetNode class.
 */

module;
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <ImNodeFlow.h>
#include <imgui_zoomable_image.h>

// For Windows OpenGL compatibility.
#ifndef APIENTRY
#define APIENTRY
#endif
export module yellow_mug.nodes:target;

import std;
import yellow_mug.core;

export namespace yellow_mug
{

/**
 * @brief Node wrapping the visualization and output of the graph in a dedicated window.
 *
 * @details
 * Inherits publicly from @ref ImFlow::BaseNode. @ref draw() presents controls
 * within the graph and manages rendering a separate dockable image viewer window
 * using @ref imgui_zoomable_image.
 *
 * @see Frame
 */
class TargetNode : public ImFlow::BaseNode
{
public:
	/**
	 * @brief Constructs the node and registers the input pin.
	 */
	TargetNode();

	/**
	 * @brief Destroys the node and deletes any managed OpenGL textures.
	 */
	~TargetNode() override;

	/**
	 * @brief Draws the node controls inside the node graph.
	 *
	 * @details
	 * Presents a "Run" button to trigger recursive pipeline evaluation, a
	 * "View Image" button to open the floating window, and static text
	 * showing frame dimensions.
	 */
	void draw() override;

	/**
	 * @brief Draws the standalone image viewer window.
	 *
	 * @details
	 * Renders a dockable floating window. Uses @ref imgui_zoomable_image for
	 * pan and zoom controls and overlays the frame on an aligned GPU checkerboard.
	 */
	void draw_viewer();

	/**
	 * @brief Sets the parent context pointer used to draw floating windows.
	 *
	 * @param context Pointer to the outer Dear ImGui context.
	 */
	void set_outer_context(ImGuiContext* context) noexcept
	{
		m_outer_context = context;
	}

private:
	/**
	 * @brief Uploads the pixel data of @p frame to the OpenGL texture.
	 *
	 * @param frame The frame containing the pixel data to upload.
	 */
	void upload_texture(const Frame& frame);

	/**
	 * @brief Lazily generates and configures the pre-tiled checkerboard texture.
	 */
	void init_checkerboard();

	/// @brief Unique identifier of the managed OpenGL texture.
	GLuint m_texture_id{0};

	/// @brief Unique identifier of the procedurally generated tiled checkerboard texture.
	GLuint m_checkerboard_texture_id{0};

	/// @brief Stores the previously bound sampler ID during drawing callbacks.
	GLuint m_last_sampler{0};

	/// @brief Function pointer signature for modern OpenGL sampler binding.
	using PFNGLBINDSAMPLERPROC = void (APIENTRY *)(GLuint, GLuint);

	/// @brief Statically cached function pointer to the OpenGL glBindSampler function.
	static inline PFNGLBINDSAMPLERPROC s_glBindSamplerFn{nullptr};

	/**
	 * @brief Shared pointer to the most recently evaluated frame.
	 *
	 * @details
	 * Holds `nullptr` until a successful pipeline evaluation occurs.
	 */
	std::shared_ptr<const Frame> m_displayed{nullptr};

	/// @brief Zoom and pan state used by @ref imgui_zoomable_image.
	ImGuiImage::State m_zoom_state{ .maintainAspectRatio = true };

	/// @brief Controls whether the separate image viewer window is open.
	bool m_viewer_open{false};

	/// @brief Controls whether to draw a checkerboard pattern behind transparent pixels.
	bool m_show_checkerboard{true};

	/// @brief Pointer to the main application's outer Dear ImGui context.
	ImGuiContext* m_outer_context{nullptr};
};

TargetNode::TargetNode()
{
	setTitle("Target");
	addIN_uid<std::shared_ptr<const Frame>>(0, "in", nullptr, ImFlow::ConnectionFilter::None());

	if (s_glBindSamplerFn == nullptr) [[unlikely]]
	{
		s_glBindSamplerFn = reinterpret_cast<PFNGLBINDSAMPLERPROC>(glfwGetProcAddress("glBindSampler"));
	}
}

TargetNode::~TargetNode()
{
	if (m_texture_id != 0)
	{
		glDeleteTextures(1, &m_texture_id);
	}
	if (m_checkerboard_texture_id != 0)
	{
		glDeleteTextures(1, &m_checkerboard_texture_id);
	}
}

void TargetNode::init_checkerboard()
{
	if (m_checkerboard_texture_id != 0)
	{
		return;
	}

	// Acquire an OpenGL texture.
	glGenTextures(1, &m_checkerboard_texture_id);
	glBindTexture(GL_TEXTURE_2D, m_checkerboard_texture_id);

	// 4 pixel square defining the checkerboard colors.
	constexpr std::array pixels = std::to_array<std::uint8_t>({
		0xBF, 0xBF, 0xBF, 0xFF, /* #BFBFBFFF */
		0x7F, 0x7F, 0x7F, 0xFF, /* #7F7F7FFF */

		0x7F, 0x7F, 0x7F, 0xFF, /* #7F7F7FFF */
		0xBF, 0xBF, 0xBF, 0xFF, /* #BFBFBFFF */
	});

	// GL_UNPACK_ROW_LENGTH is unavailable on WebGL.
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	// The pixels are tightly packed, without any padding.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

	// The pixels are 1 byte each.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Upload the pixels to OpenGL as a texture.
	glTexImage2D(
		GL_TEXTURE_2D,
		0,                // mipmap level
		GL_RGBA,          // internal pixel format
		2, 2,             // width and height in pixels
		0,                // legacy border - must be 0
		GL_RGBA,          // the pixel format of the input
		GL_UNSIGNED_BYTE, // the pixel data type
		pixels.data()     // a pointer to the pixels
	);

	// Use nearest filtering for magnifying and minifying the texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Repeat the texture both vertically and horizontally
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Unbind the texture, so ImGui does not mess with the checkerboard.
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TargetNode::upload_texture(const Frame& frame)
{
	if (m_texture_id == 0)
	{
		glGenTextures(1, &m_texture_id);
	}

	const auto& [width, height] = frame.dimensions();
	glBindTexture(GL_TEXTURE_2D, m_texture_id);

	// GL_UNPACK_ROW_LENGTH is unavailable on WebGL
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	// The pixels are tightly packed, without any padding.
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

	// The pixels are 1 byte each.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,                                 // mipmap level
		GL_RGBA,                           // internal pixel format
		static_cast<GLsizei>(width),
		static_cast<GLsizei>(height),
		0,                                 // legacy border - must be 0
		GL_RGBA,                           // the pixel format of the input
		GL_UNSIGNED_BYTE,                  // the pixel data type
		frame.bytes().data()               // a pointer to the pixels
	);

	// Use nearest filtering for magnifying and minifying the texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Define the transparent solid color.
	constexpr std::array border_color{ 0.0f, 0.0f, 0.0f, 0.0f };

	// Fill the remaining area with solid color (transparency).
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.data());

	// Unbind the texture, so ImGui does not mess with the texture.
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TargetNode::draw()
{
	if (ImGui::Button("Run"))
	{
		auto frame = getInVal<std::shared_ptr<const Frame>>(0);
		if (frame && !frame->dimensions().empty())
		{
			upload_texture(*(m_displayed = frame));
			m_viewer_open = true;
		}
		else
		{
			m_displayed = nullptr;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("View Image"))
	{
		m_viewer_open = true;
	}

	if (m_displayed)
	{
		const auto& [width, height] = m_displayed->dimensions();
		ImGui::Text("Size: %zu x %zu", width, height);
	}
	else
	{
		ImGui::TextUnformatted("Status: No image");
	}

	if (m_viewer_open && m_outer_context != nullptr)
	{
		ImGuiContext* inner_ctx = ImGui::GetCurrentContext();
		ImGui::SetCurrentContext(m_outer_context);

		draw_viewer();

		ImGui::SetCurrentContext(inner_ctx);
	}
}

void TargetNode::draw_viewer()
{
	if (!m_viewer_open)
	{
		return;
	}

	init_checkerboard();

	if (ImGui::Begin("Image Viewer", &m_viewer_open, ImGuiWindowFlags_NoScrollbar))
	{
		if (m_displayed && m_texture_id != 0)
		{
			const auto& [width, height] = m_displayed->dimensions();
			m_zoom_state.textureSize = ImVec2(static_cast<float>(width), static_cast<float>(height));

			ImGui::Checkbox("Transparent Checkerboard", &m_show_checkerboard);
			ImGui::SameLine();
			ImGui::Checkbox("Maintain Aspect Ratio", &m_zoom_state.maintainAspectRatio);

			ImVec2 display_size = ImGui::GetContentRegionAvail();

			if (display_size.x > 0.0f && display_size.y > 0.0f)
			{
				ImVec2 uv0(0.0f, 0.0f);
				ImVec2 uv1(1.0f, 1.0f);

				// Calculate padding if needed so the image does not clip.
				if (m_zoom_state.maintainAspectRatio)
				{
					float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
					float canvas_aspect = display_size.x / display_size.y;

					if (canvas_aspect > aspect_ratio)
					{
						float scale = canvas_aspect / aspect_ratio;
						uv0.x = 0.5f - 0.5f * scale;
						uv1.x = 0.5f + 0.5f * scale;
					}
					else
					{
						float scale = aspect_ratio / canvas_aspect;
						uv0.y = 0.5f - 0.5f * scale;
						uv1.y = 0.5f + 0.5f * scale;
					}
				}

				ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

				// Scoped helper lambdas to manage OpenGL sampler state.
				// These are static, constexpr, and noexcept for optimal execution as C-style callbacks.
				static constexpr auto unbind_callback = [](const ImDrawList*, const ImDrawCmd* cmd) noexcept
				{
					// Store the sampler
					auto* node = static_cast<TargetNode*>(cmd->UserCallbackData);
					glGetIntegerv(GL_SAMPLER_BINDING, reinterpret_cast<GLint*>(&node->m_last_sampler));

					// Unbind the sampler
					if (s_glBindSamplerFn != nullptr)
					{
						s_glBindSamplerFn(0, 0);
					}
				};

				static constexpr auto restore_callback = [](const ImDrawList*, const ImDrawCmd* cmd) noexcept
				{
					// Restore the saved sampler.
					auto* node = static_cast<TargetNode*>(cmd->UserCallbackData);
					if (s_glBindSamplerFn != nullptr)
					{
						s_glBindSamplerFn(0, node->m_last_sampler);
					}
				};

				if (m_show_checkerboard && m_checkerboard_texture_id != 0)
				{
					ImDrawList* draw_list = ImGui::GetWindowDrawList();

					// Callback magic to render the checkerboard using "raw" OpenGL and hardware acceleration.
					draw_list->AddCallback(unbind_callback, this);

					// Render hardware-tiled 2x2px checkerboard
					draw_list->AddImage(
						static_cast<ImTextureID>(m_checkerboard_texture_id),
						canvas_pos,
						ImVec2(canvas_pos.x + display_size.x, canvas_pos.y + display_size.y),
						ImVec2(0.0f, 0.0f),
						ImVec2(display_size.x / 32.0f, display_size.y / 32.0f)
					);

					// Restore the previously bound sampler.
					draw_list->AddCallback(restore_callback, this);
				}

				// Inject the unbind callback into the child draw list.
				// This disables ImGui's sampler specifically for this widget,
				// allowing the use of other samplers and GL_CLAMP_TO_BORDER.
				ImGui::BeginChild("ImageRegion", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);
				ImDrawList* child_draw_list = ImGui::GetWindowDrawList();
				child_draw_list->AddCallback(unbind_callback, this);
				ImGui::EndChild();

				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

				// Trick to stretch the image.
				// Aspect ratio is maintained manually.
				bool user_maintain = m_zoom_state.maintainAspectRatio;
				m_zoom_state.maintainAspectRatio = false;

				// Workaround. Uses default values in the 7-parameter function, as the 5-parameter one simply ignores uv0 and uv1.
				ImGuiImage::Zoomable(
					static_cast<ImTextureID>(m_texture_id),
					display_size,
					uv0,
					uv1,
					ImGuiImage::kDefaultBackgroundColor,
					ImGuiImage::kDefaultTintColor,
					&m_zoom_state
				);

				m_zoom_state.maintainAspectRatio = user_maintain;

				ImGui::PopStyleColor();

				// Inject the restore callback into the child draw list
				ImGui::BeginChild("ImageRegion", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove);
				child_draw_list = ImGui::GetWindowDrawList();
				child_draw_list->AddCallback(restore_callback, this);
				ImGui::EndChild();
			}
		}
		else
		{
			ImGui::TextUnformatted("Please run the pipeline to view the output.");
		}
	}
	ImGui::End();
}

} // namespace yellow_mug
