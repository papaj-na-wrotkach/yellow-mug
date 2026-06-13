module;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <ImNodeFlow.h>
export module yellow_mug.app;
import std;
import yellow_mug.nodes;

namespace yellow_mug {
	export class App {
	public:
		App() {
			init_window();
			init_imgui();
			setup_context_menu();
		}

		~App() {
			cleanup();
		}

		void run() {
			while (!glfwWindowShouldClose(m_window)) {
				glfwPollEvents();

				// Start the Dear ImGui frame
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				// Enable docking
				ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

				// Draw node editor window
				ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
				if (ImGui::Begin("Node Editor")) {
					if (ImGuiContext* inner_ctx = m_graph.getGrid().getRawContext(); inner_ctx) {
						ImGui::SetCurrentContext(inner_ctx);
						ImGui::GetPlatformIO() = m_context->PlatformIO;
						ImGui::SetCurrentContext(m_context);
					}
					m_graph.update();
				}

				ImGui::End();

				// Render
				ImGui::Render();
				int display_w, display_h;
				glfwGetFramebufferSize(m_window, &display_w, &display_h);
				glViewport(0, 0, display_w, display_h);

				glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
				glfwSwapBuffers(m_window);
			}
		}

	private:
		ImGuiContext* m_context{nullptr};
		GLFWwindow* m_window{nullptr};
		ImFlow::ImNodeFlow m_graph;

		void setup_context_menu() {
			m_graph.rightClickPopUpContent([this](ImFlow::BaseNode* node) {
				if (node == nullptr) {
					if (ImGui::Selectable("Source")) {
						m_graph.placeNode<SourceProcessorNode>();
					}
					if (ImGui::Selectable("Output")) {
						auto target = m_graph.placeNode<TargetNode>();
						target->set_outer_context(m_context);
					}
				} else {
					if (ImGui::Selectable("Delete Node")) {
						if (node->isSelected()) {
							std::ranges::for_each(
								m_graph.getNodes()
									| std::views::values
									| std::views::filter(&ImFlow::BaseNode::isSelected),
								std::bind_front(&ImFlow::BaseNode::destroy));
						} else {
							node->destroy();
						}
					}
				}
			});
		}

		void init_window() {
			if (!glfwInit()) {
				throw std::runtime_error("Failed to initialize GLFW");
			}

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

			m_window = glfwCreateWindow(1280, 720, "Image Filter Pipeline", nullptr, nullptr);
			if (!m_window) {
				glfwTerminate();
				throw std::runtime_error("Failed to create GLFW window");
			}

			glfwMakeContextCurrent(m_window);
			glfwSwapInterval(1); // Enable vsync
		}

		void init_imgui() {
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			m_context = ImGui::GetCurrentContext();
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
			ImGui::StyleColorsDark();

			ImGui_ImplGlfw_InitForOpenGL(m_window, true);
			ImGui_ImplOpenGL3_Init("#version 330");
		}

		void cleanup() {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();

			ImGui::DestroyContext();

			if (m_window) glfwDestroyWindow(m_window);
			glfwTerminate();
		}
	};
}
