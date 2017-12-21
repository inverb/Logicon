//
// Created by rufus on 12.12.17.
//

#include "app.h"

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <imgui_impl_glfw_gl3.h>

#include <iostream> // for error printing

namespace Logicon {

    std::string App::APP_TITLE = "Logicon";

    void error_callback(int error, const char* description) {
        fprintf(stderr, "Error %d: %s\n", error, description);
    }

    bool App::init() {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW.\n";
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        this->window = glfwCreateWindow(1280, 720, App::APP_TITLE.c_str(), NULL, NULL);
        glfwMakeContextCurrent(this->window);
        glfwSwapInterval(1); // Enable vsync
        gl3wInit();

        if ((ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
          ||(iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION)
          ||(ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION)) {
            std::cerr << "DevIL versions are different... Exiting.\n";
            return false;
        }

        //// Initialise all DevIL functionality
        ilutRenderer(ILUT_OPENGL);
        ilInit();
        iluInit();
        ilutInit();
        ilutRenderer(ILUT_OPENGL);

        // Setup ImGui binding
        ImGui_ImplGlfwGL3_Init(this->window, true);

        this->state = State::UNINITIALIZED;

        /*
         * Add default app font
         * This must happen before initialization of any other app components
         * since default app font must be defined as the first one.
         */
        ImGuiIO& io = ImGui::GetIO();
        static ImFontConfig defaultFontConfig = ImFontConfig();
        io.Fonts->AddFontFromFileTTF("../dependencies/imgui/extra_fonts/Roboto-Medium.ttf", 15.0f, &defaultFontConfig);

        ImGui::GetStyle().WindowRounding = 0.0f;

        this->menuWidget = MenuWidget();
        if(!this->menuWidget.init(this->window))
            return false;

        this->blocksWidget = BlocksWidget();
        if(!this->blocksWidget.init(this->window))
            return false;

        this->footerWidget = FooterWidget();
        if(!this->footerWidget.init(this->window))
            return false;

        return true;
    }

    void App::run() {
        if(!init()) {
            std::cerr << "Failed to initialize the app.\n";
            exit(-1);
        }

        while (!glfwWindowShouldClose(this->window)) {
            glfwPollEvents();
            ImGui_ImplGlfwGL3_NewFrame();

            this->render_ui();

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui::Render();
            glfwSwapBuffers(window);
        }

        if(!close()) {
            std::cerr << "Failed to properly close the app.\n";
            exit(-1);
        }
    }

    bool App::close() {
        if(!this->menuWidget.close())
            return false;

        if(!this->blocksWidget.close())
            return false;

        if(!this->footerWidget.close())
            return false;

        ImGui_ImplGlfwGL3_Shutdown();
        glfwTerminate();

        return true;
    }

    ID App::nextID() {
        static unsigned int next_id = 0;
        return next_id++;
    }

    void App::render_ui() {
        int canvas_w, canvas_h;
        glfwGetFramebufferSize(window, &canvas_w, &canvas_h);

        /*
         * Menu Widget
         */
        const ImVec2 menu_widget_pos = ImVec2(
                UI::MARGIN,
                UI::MARGIN
        );
        const ImVec2 menu_widget_size = ImVec2(
                canvas_w - 2*UI::MARGIN - UI::BLOCKS_WIDGET_WIDTH - UI::MARGIN,
                UI::MENU_WIDGET_HEIGHT
        );
        this->menuWidget.render_ui(menu_widget_pos, menu_widget_size);

        /*
         * Blocks Widget
         */
        const ImVec2 blocks_widget_pos = ImVec2(
                canvas_w - UI::MARGIN - UI::BLOCKS_WIDGET_WIDTH,
                UI::MARGIN
        );
        const ImVec2 blocks_widget_size = ImVec2(
                UI::BLOCKS_WIDGET_WIDTH,
                canvas_h - (UI::MARGIN + UI::FOOTER_WIDGET_HEIGHT + UI::MARGIN)
        );
        this->blocksWidget.render_ui(blocks_widget_pos, blocks_widget_size);

        /*
         * Footer Widget
         */
        const ImVec2 footer_widget_pos = ImVec2(
                0,
                canvas_h - (UI::FOOTER_WIDGET_HEIGHT)
        );
        const ImVec2 footer_widget_size = ImVec2(
                canvas_w,
                UI::FOOTER_WIDGET_HEIGHT
        );
        this->footerWidget.render_ui(footer_widget_pos, footer_widget_size);
    }

} // namespace Logicon