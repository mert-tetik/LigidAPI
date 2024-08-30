// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h> // Will drag system OpenGL headers

#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>

#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#endif

#define LIGIDAPI_OPENGL_IMPLEMENTATION
#include "../Headers/OpenGL_specific.h" // Define functions there

#include "../Headers/LigidAPI.h"
#include "../Headers/Canvas.h"
#include "../Headers/Filter.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
}


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool mouse_clicked = false;
bool mouse_released = false;
bool mouse_pressed = false;
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Check if the left mouse button was pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouse_clicked = true;
            mouse_pressed = true;
        } else if (action == GLFW_RELEASE) {
            mouse_pressed = false;
        }
    }
}

ImVec2 display_pos;
ImVec2 display_scale;
void RenderImageWithTracking(ImTextureID texture_id, const ImVec2& image_size) {

    // Render the image
    ImVec2 image_pos = ImGui::GetCursorScreenPos(); // Get the position where the image will be rendered
    ImGui::Image(texture_id, image_size);

    // After rendering, get the size of the last item (which is the image)
    display_scale = ImGui::GetItemRectSize(); // Get the actual size of the image
    display_pos = ImGui::GetItemRectMin();  // Get the actual position of the image
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 400 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "LigidAPI Interface", nullptr, nullptr);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }  

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImVec4 theme_color = ImVec4(0.89f, 0.73f, 0.31f, 1.f); 

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = theme_color;
    colors[ImGuiCol_FrameBgActive]          = theme_color;
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = theme_color;
    colors[ImGuiCol_SliderGrab]             = theme_color;
    colors[ImGuiCol_SliderGrabActive]       = theme_color;
    colors[ImGuiCol_Button]                 = theme_color;
    colors[ImGuiCol_ButtonHovered]          = theme_color;
    colors[ImGuiCol_ButtonActive]           = theme_color;
    colors[ImGuiCol_Header]                 = theme_color;
    colors[ImGuiCol_HeaderHovered]          = theme_color;
    colors[ImGuiCol_HeaderActive]           = theme_color;
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    colors[ImGuiCol_SeparatorHovered]       = theme_color;
    colors[ImGuiCol_SeparatorActive]        = theme_color;
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
    colors[ImGuiCol_ResizeGripHovered]      = theme_color;
    colors[ImGuiCol_ResizeGripActive]       = theme_color;
    colors[ImGuiCol_TabDimmedSelectedOverline] = theme_color;
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = theme_color;
    colors[ImGuiCol_TableBorderStrong]      = theme_color;
    colors[ImGuiCol_TableBorderLight]       = theme_color;
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    colors[ImGuiCol_TextSelectedBg]         = theme_color;
    colors[ImGuiCol_DragDropTarget]         = theme_color;
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;

    int my_image_width = 0;
    int my_image_height = 0;
    GLuint my_image_texture = 0;
    bool ret = LoadTextureFromFile("./Main/Example/Resources/image.jpg", &my_image_texture, &my_image_width, &my_image_height);
    IM_ASSERT(ret);
    
    // Creating filter texture
    GLuint filter_buffer = 0;
    int filter_buffer_width = 1024;
    int filter_buffer_height = 1024;
    int filter_buffer_channels = 4;
    {
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &filter_buffer);
        glBindTexture(GL_TEXTURE_2D, filter_buffer);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, filter_buffer_width, filter_buffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    
    // Init ligidapi
    LigidAPI_init("OPENGL", "#version 400 core");

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        mouse_clicked = false;
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static bool x = false;
            ImGui::Begin("LigidAPI Interface", &x, 4);
            
            // Get the size of the viewport
            ImVec2 viewportSize = ImGui::GetIO().DisplaySize;

            // Set the window size and position to cover the entire window
            ImGui::SetWindowSize(viewportSize);
            ImGui::SetWindowPos(ImVec2(0, 0));

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

            const int features_items_count = 4;             
            const char *const feature_items[features_items_count] = {"Filtering", "Mixing", "Painting", "Rendering"};
            static int selected_feature = 0;
            ImGui::ListBox("Features", &selected_feature, feature_items, features_items_count, -1);               // Display some text (you can use a format strings too)
            
            GLuint displayed_texture = filter_buffer;
            GLuint displayed_texture_w = my_image_width;
            GLuint displayed_texture_h = my_image_height;

            if(selected_feature == 0){
                const int filter_items_count = 4;             
                const char *const filter_items[filter_items_count] = {"Invert", "HSV", "RGB", "Brightness"};
                static int selected_filter = 0;
                ImGui::ListBox("Filters", &selected_filter, filter_items, filter_items_count, -1);               // Display some text (you can use a format strings too)

                LigidFilter* image_filter = nullptr;
                float filter_var_1 = 0.;
                float filter_var_2 = 0.;
                float filter_var_3 = 0.;
                float filter_var_4 = 0.;
                float filter_var_5 = 0.;

                if(selected_filter == 0){
                    static bool invert_filter_invert_red;
                    static bool invert_filter_invert_green;
                    static bool invert_filter_invert_blue;
                    static bool invert_filter_invert_alpha;
                    ImGui::Checkbox("Invert Red", &invert_filter_invert_red);
                    ImGui::Checkbox("Invert Green", &invert_filter_invert_green);
                    ImGui::Checkbox("Invert Blue", &invert_filter_invert_blue);
                    ImGui::Checkbox("Invert Alpha", &invert_filter_invert_alpha);
                
                    image_filter = &LigidAPI_filter_invert;

                    filter_var_1 = (float)invert_filter_invert_red;
                    filter_var_2 = (float)invert_filter_invert_green;
                    filter_var_3 = (float)invert_filter_invert_blue;
                    filter_var_4 = (float)invert_filter_invert_alpha;
                }
                
                if(image_filter != nullptr){
                    // Filter texture via ligidapi
                    LigidAPI_apply_filter(
                                                                my_image_texture, 
                                                                my_image_width, 
                                                                my_image_height,
                                                                filter_buffer,
                                                                filter_buffer_width,
                                                                filter_buffer_height,
                                                                &LigidAPI_filter_invert, 
                                                                filter_var_1, 
                                                                filter_var_2, 
                                                                filter_var_3, 
                                                                filter_var_4, 
                                                                filter_var_5
                                                            );
                }
            }
            else if(selected_feature == 2){
                static ImVec4 painting_color;
                ImGui::ColorEdit4("Painting Color", (float*)&painting_color);

                // Create the canvas
                static LigidCanvas* canvas = LigidAPI_create_canvas(1024, 1024, 4, LigidAPI_get_color(0.f, 0.f, 0.f, 0.f));
    
                static double cursor_x = 0, cursor_y = 0, last_cursor_x = 0, last_cursor_y = 0;
                
                last_cursor_x = cursor_x;
                last_cursor_y = cursor_y;
                
                if(mouse_pressed){
                    glfwGetCursorPos(window, &cursor_x, &cursor_y);
                }                

                LigidArea render_area;
                render_area.pos_x = display_pos.x;
                render_area.pos_y = display_pos.y;
                render_area.width = display_scale.x;
                render_area.height = display_scale.y;
                LigidArea screen_area;
                screen_area.pos_x = 0;
                screen_area.pos_y = 0;
                screen_area.width = canvas->width;
                screen_area.height = canvas->height;

                LigidBrush brush = LigidAPI_create_brush(0.01f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0);
                LigidStroke stroke = LigidAPI_project_stroke_to_render_area(LigidAPI_get_stroke(cursor_x, cursor_y, last_cursor_x, last_cursor_y), render_area, screen_area);
                LigidAPI_paint_canvas(canvas, brush, stroke, LigidAPI_get_color(painting_color.x, painting_color.y, painting_color.z, painting_color.w));    

                displayed_texture = canvas->opengl_texture_buffer_ID;
                displayed_texture_w = 1024;
                displayed_texture_h = 1024;

                //LigidAPI_delete_canvas(canvas);
            }

            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);
            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            //ImGui::SameLine();

            RenderImageWithTracking((void*)(intptr_t)displayed_texture, ImVec2(displayed_texture_w/2, displayed_texture_h/2));

            // Check if the image is being hovered
            bool isImageHovered = ImGui::IsItemHovered();

            // Disable dragging if image is hovered
            if (isImageHovered) {
                ImGui::SetWindowPos(ImVec2(100, 100), ImGuiCond_Once); // Set position conditionally
                ImGui::SetWindowSize(ImVec2(400, 300), ImGuiCond_Once); // Set size conditionally
            } else {
                // Optionally, you can handle dragging here
            }

            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(1., 1., 1., 1.);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
