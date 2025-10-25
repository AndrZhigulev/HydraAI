/**
 * @file gui.cpp
 * @brief ImGui interface for HydraAI Worker
 *
 * This implements a beautiful, user-friendly GUI for the worker client using ImGui.
 * Shows real-time training progress, token balance, and system statistics.
 */

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <cmath>

namespace hydra {
namespace worker {

/**
 * @class WorkerGUI
 * @brief Main GUI class for the worker client
 */
class WorkerGUI {
public:
    struct Stats {
        std::string user_id;
        std::string status = "Disconnected";
        double token_balance = 0.0;
        int tasks_completed = 0;
        int tasks_in_progress = 0;

        // Training metrics
        std::vector<float> loss_history;
        float current_loss = 0.0f;
        float current_progress = 0.0f;  // 0.0 to 1.0

        // System info
        float cpu_usage = 0.0f;
        float memory_usage = 0.0f;
        float gpu_usage = 0.0f;  // If GPU available
    };

    WorkerGUI() : window_(nullptr), stats_{} {}

    ~WorkerGUI() {
        shutdown();
    }

    /**
     * @brief Initialize the GUI window
     * @param title Window title
     * @param width Window width
     * @param height Window height
     * @return true if successful
     */
    bool initialize(const std::string& title = "HydraAI Worker",
                   int width = 1280, int height = 720) {
        // Initialize GLFW
        if (!glfwInit()) {
            return false;
        }

        // GL 3.3 + GLSL 330
        const char* glsl_version = "#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // Create window
        window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window_) {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable vsync

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard navigation
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable docking

        // Setup style - dark theme
        ImGui::StyleColorsDark();
        customize_style();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window_, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        return true;
    }

    /**
     * @brief Main render loop
     * @return false when window should close
     */
    bool render_frame() {
        // Poll events
        glfwPollEvents();

        if (glfwWindowShouldClose(window_)) {
            return false;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render GUI
        render_gui();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window_);

        return true;
    }

    /**
     * @brief Update GUI statistics
     * @param stats New statistics
     */
    void update_stats(const Stats& stats) {
        stats_ = stats;

        // Keep loss history limited
        if (stats_.loss_history.size() > 500) {
            stats_.loss_history.erase(stats_.loss_history.begin());
        }
    }

    /**
     * @brief Shutdown GUI
     */
    void shutdown() {
        if (window_) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(window_);
            glfwTerminate();

            window_ = nullptr;
        }
    }

private:
    GLFWwindow* window_;
    Stats stats_;

    /**
     * @brief Customize ImGui style for a modern look
     */
    void customize_style() {
        ImGuiStyle& style = ImGui::GetStyle();

        // Rounding
        style.WindowRounding = 5.0f;
        style.FrameRounding = 3.0f;
        style.GrabRounding = 2.0f;
        style.ScrollbarRounding = 3.0f;

        // Spacing
        style.WindowPadding = ImVec2(10, 10);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(8, 6);

        // Colors - Professional dark blue theme
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.13f, 1.0f);
        colors[ImGuiCol_Header] = ImVec4(0.2f, 0.3f, 0.5f, 0.6f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.4f, 0.6f, 0.8f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.5f, 0.7f, 1.0f);
        colors[ImGuiCol_Button] = ImVec4(0.2f, 0.3f, 0.5f, 0.6f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.4f, 0.6f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.5f, 0.7f, 1.0f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.5f, 0.8f, 0.3f, 1.0f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.2f, 0.3f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.3f, 0.5f, 1.0f);
    }

    /**
     * @brief Render the main GUI
     */
    void render_gui() {
        // Fullscreen dockspace
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        // DockSpace
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window_, GLFW_TRUE);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Statistics", nullptr, true);
                ImGui::MenuItem("Training Progress", nullptr, true);
                ImGui::MenuItem("System Monitor", nullptr, true);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    // Show about dialog
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();

        // Render individual panels
        render_status_panel();
        render_training_panel();
        render_system_panel();
        render_log_panel();
    }

    /**
     * @brief Render status panel showing connection and token balance
     */
    void render_status_panel() {
        ImGui::Begin("Status");

        // User info
        ImGui::Text("User ID: %s", stats_.user_id.c_str());

        // Connection status with colored indicator
        ImVec4 status_color = stats_.status == "Connected"
            ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f)  // Green
            : ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red

        ImGui::TextColored(status_color, "Status: %s", stats_.status.c_str());

        ImGui::Separator();

        // Token balance (large, prominent)
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);  // Default font
        ImGui::Text("Token Balance:");
        ImGui::PopFont();

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%.2f HYDRA", stats_.token_balance);

        ImGui::Separator();

        // Tasks summary
        ImGui::Text("Tasks Completed: %d", stats_.tasks_completed);
        ImGui::Text("Tasks in Progress: %d", stats_.tasks_in_progress);

        ImGui::Separator();

        // Action buttons
        if (stats_.status == "Connected") {
            if (ImGui::Button("Pause Training")) {
                // Pause training
            }
        } else {
            if (ImGui::Button("Connect to Coordinator")) {
                // Attempt connection
            }
        }

        ImGui::End();
    }

    /**
     * @brief Render training progress panel
     */
    void render_training_panel() {
        ImGui::Begin("Training Progress");

        if (stats_.tasks_in_progress > 0) {
            // Current task progress
            ImGui::Text("Current Task Progress:");
            ImGui::ProgressBar(stats_.current_progress, ImVec2(-1, 0), "");

            ImGui::Separator();

            // Loss curve
            ImGui::Text("Training Loss:");
            if (!stats_.loss_history.empty()) {
                ImGui::PlotLines("##loss", stats_.loss_history.data(),
                               stats_.loss_history.size(),
                               0, nullptr, FLT_MAX, FLT_MAX,
                               ImVec2(0, 80));
            }

            // Current loss value
            ImGui::Text("Current Loss: %.4f", stats_.current_loss);
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                             "No active training tasks");
        }

        ImGui::End();
    }

    /**
     * @brief Render system resource monitor
     */
    void render_system_panel() {
        ImGui::Begin("System Monitor");

        // CPU usage
        ImGui::Text("CPU Usage:");
        ImGui::ProgressBar(stats_.cpu_usage / 100.0f, ImVec2(-1, 0),
                          std::to_string((int)stats_.cpu_usage).append("%").c_str());

        // Memory usage
        ImGui::Text("Memory Usage:");
        ImGui::ProgressBar(stats_.memory_usage / 100.0f, ImVec2(-1, 0),
                          std::to_string((int)stats_.memory_usage).append("%").c_str());

        // GPU usage (if available)
        if (stats_.gpu_usage > 0.0f) {
            ImGui::Text("GPU Usage:");
            ImGui::ProgressBar(stats_.gpu_usage / 100.0f, ImVec2(-1, 0),
                              std::to_string((int)stats_.gpu_usage).append("%").c_str());
        }

        ImGui::Separator();

        // Performance tips
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Performance Tips:");
        ImGui::BulletText("Close other applications to free resources");
        ImGui::BulletText("Ensure good cooling for sustained performance");

        ImGui::End();
    }

    /**
     * @brief Render activity log panel
     */
    void render_log_panel() {
        ImGui::Begin("Activity Log");

        // Sample log messages (in real implementation, this would be a scrolling log)
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "[INFO]");
        ImGui::SameLine();
        ImGui::Text("Connected to coordinator");

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "[INFO]");
        ImGui::SameLine();
        ImGui::Text("Received training task: task-abc123");

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "[TRAIN]");
        ImGui::SameLine();
        ImGui::Text("Training epoch 1/3...");

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "[SUCCESS]");
        ImGui::SameLine();
        ImGui::Text("Task completed! Earned 1.0 HYDRA tokens");

        ImGui::End();
    }
};

} // namespace worker
} // namespace hydra
