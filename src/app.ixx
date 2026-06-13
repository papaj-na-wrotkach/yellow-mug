module;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <ImNodeFlow.h>
export module yellow_mug.app;
import std;

namespace yellow_mug {
    export class App {
    public:
        App() {
            init_window();
            init_imgui();
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
        GLFWwindow* m_window{nullptr};
        ImFlow::ImNodeFlow m_graph;

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
