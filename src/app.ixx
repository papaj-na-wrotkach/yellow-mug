/**
 * @file app.ixx
 * @brief Declaration and definition of the @ref yellow_mug::App class.
 */

module;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <ImNodeFlow.h>

export module yellow_mug.app;

import std;
import yellow_mug.nodes;

export namespace yellow_mug
{

/**
 * @brief Top-level application class that owns the GLFW window and the node graph.
 *
 * @details
 * `App` encapsulates the full lifecycle of the application:
 * - creating and destroying the GLFW window and OpenGL context,
 * - initialising and shutting down Dear ImGui and its backends,
 * - owning the @ref ImFlow::ImNodeFlow graph instance,
 * - running the main event loop via @ref run().
 *
 * Construction calls @ref init_window() and @ref init_imgui(). Both methods
 * throw `std::runtime_error` on failure, so no separate validity check is
 * required after construction.
 */
class App
{
public:
	/**
	 * @brief Constructs the application, creating the window and initialising Dear ImGui.
	 *
	 * @throws std::runtime_error If GLFW initialisation or window creation fails.
	 */
	App()
	{
		init_window();
		init_imgui();
		setup_context_menu();
	}

	/**
	 * @brief Destroys the application, releasing all GLFW and ImGui resources.
	 */
	~App()
	{
		cleanup();
	}

	/**
	 * @brief Runs the main event loop until the window is closed.
	 *
	 * @details
	 * On each iteration, polls GLFW events, begins a new Dear ImGui frame,
	 * updates the @ref ImFlow::ImNodeFlow graph, renders the ImGui draw data
	 * via the OpenGL 3 backend, and swaps the GLFW buffers.
	 */
	void run()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

			ImGui::SetNextWindowSize(ImVec2{1024, 768}, ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Node Editor"))
			{
				if (auto* inner_ctx = m_graph.getGrid().getRawContext(); inner_ctx)
				{
					ImGui::SetCurrentContext(inner_ctx);
					ImGui::GetPlatformIO() = m_context->PlatformIO;
					ImGui::SetCurrentContext(m_context);
				}
				m_graph.update();
			}

			ImGui::End();

			ImGui::Render();
			auto display_w{0};
			auto display_h{0};
			glfwGetFramebufferSize(m_window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);

			glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				auto* current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(current_context);
			}
			glfwSwapBuffers(m_window);
		}
	}

private:
	/// @brief Pointer to the main Dear ImGui context.
	ImGuiContext* m_context{nullptr};

	/// @brief Pointer to the application's GLFW window.
	GLFWwindow* m_window{nullptr};

	/// @brief The ImNodeFlow graph that contains all processor nodes.
	ImFlow::ImNodeFlow m_graph;

	/**
	 * @brief Configures the right-click context menu for node placement and deletion.
	 *
	 * @details
	 * Registers a popup callback with @ref ImFlow::ImNodeFlow::rightClickPopUpContent().
	 * When invoked on empty canvas space, the popup lists all available node types
	 * as `ImGui::Selectable` items. When invoked on an existing node, it offers
	 * a "Delete Node" action.
	 */
	void setup_context_menu()
	{
		m_graph.rightClickPopUpContent([this](ImFlow::BaseNode* node)
		{
			if (node == nullptr)
			{
				if (ImGui::Selectable("Blur")) { m_graph.placeNode<BlurProcessorNode>(); }
				if (ImGui::Selectable("Crop")) { m_graph.placeNode<CropProcessorNode>(); }
				if (ImGui::Selectable("Grayscale")) { m_graph.placeNode<GrayscaleProcessorNode>(); }
				if (ImGui::Selectable("Invert")) { m_graph.placeNode<InvertProcessorNode>(); }
				if (ImGui::Selectable("Mask")) { m_graph.placeNode<MaskBlendProcessorNode>(); }
				if (ImGui::Selectable("Source")) { m_graph.placeNode<SourceProcessorNode>(); }
				if (ImGui::Selectable("Output"))
				{
					const auto target = m_graph.placeNode<TargetNode>();
					target->set_outer_context(m_context);
				}
			}
			else
			{
				if (ImGui::Selectable("Delete Node"))
				{
					if (node->isSelected())
					{
						std::ranges::for_each(
							m_graph.getNodes()
								| std::views::values
								| std::views::filter(&ImFlow::BaseNode::isSelected),
							std::bind_front(&ImFlow::BaseNode::destroy));
					}
					else
					{
						node->destroy();
					}
				}
			}
		});
	}

	/**
	 * @brief Initialises GLFW and creates the application window.
	 *
	 * @details
	 * Requests an OpenGL 3.3 core-profile context and creates a
	 * 1280×720 window titled "Image Filter Pipeline". V-sync is enabled.
	 *
	 * @throws std::runtime_error If `glfwInit()` or `glfwCreateWindow()` fails.
	 */
	void init_window()
	{
		if (!glfwInit())
		{
			throw std::runtime_error{"Failed to initialize GLFW"};
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(1280, 720, "Image Filter Pipeline", nullptr, nullptr);
		if (!m_window)
		{
			glfwTerminate();
			throw std::runtime_error{"Failed to create GLFW window"};
		}

		glfwMakeContextCurrent(m_window);
		glfwSwapInterval(1);
	}

	/**
	 * @brief Initializes Dear ImGui and its GLFW and OpenGL 3 backends.
	 *
	 * @details
	 * Creates the ImGui context, enables docking and keyboard navigation,
	 * and installs the `imgui_impl_glfw` and `imgui_impl_opengl3` backends.
	 * The OpenGL GLSL version string is `#version 330`.
	 */
	void init_imgui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		m_context = ImGui::GetCurrentContext();
		ImGui::GetIO().ConfigFlags |=
			ImGuiConfigFlags_DockingEnable |
			ImGuiConfigFlags_ViewportsEnable |
			ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(m_window, true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	/**
	 * @brief Shuts down ImGui backends, destroys the ImGui context, and terminates GLFW.
	 */
	void cleanup() const
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();

		if (m_window) glfwDestroyWindow(m_window);
		glfwTerminate();
	}
};

} // namespace yellow_mug
