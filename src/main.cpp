#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_opengl.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <bitset>
#include <string>

#include <cpu/GBCpu.h>

static SDL_Window *window = nullptr;
SDL_GLContext gl_context;
bool show_demo_window = false;
bool show_another_window = true;
bool show_register_info = true;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
ImGuiIO *io;

SDL_AppResult SDL_AppInit(void **, int, char **) {
    SDL_SetAppMetadata("GBEmulator", "1.0", "com.example.gbemulator");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

#if defined (__APPLE__)
    const char * glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    const char * glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", (int)(500 * main_scale), (int)(500 * main_scale), window_flags);
    if (window == nullptr)
    {
        SDL_Log("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        SDL_Log("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *, SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
    if (
        event->type == SDL_EVENT_QUIT ||
        (
            event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event->window.windowID == SDL_GetWindowID(window)
        )
    ) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *)
{

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);
        ImGui::Checkbox("Register Info", &show_register_info);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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

    if (show_register_info)
    {
        static GBCPU cpu;
        ImGui::Begin("Register Info", &show_register_info);
        if (ImGui::BeginTable("R16", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Reg");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Binary");
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("AF");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.AF());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<16>(cpu.AF()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("BC");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.BC());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<16>(cpu.BC()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("DE");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.DE());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<16>(cpu.DE()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("HL");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.HL());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<16>(cpu.HL()).to_string().c_str());
            ImGui::EndTable();
        }
        if (ImGui::BeginTable("R8", 3, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Reg");
            ImGui::TableSetupColumn("Value");
            ImGui::TableSetupColumn("Binary");
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("A");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.A());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.A()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("F");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.F());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.F()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("B");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.B());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.B()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("C");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.C());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.C()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("D");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.D());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.D()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("E");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.E());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.E()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("H");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.H());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.H()).to_string().c_str());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("L");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.L());
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", std::bitset<8>(cpu.L()).to_string().c_str());
            ImGui::EndTable();
        }
        if (ImGui::BeginTable("Flags", 2, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Flag");
            ImGui::TableSetupColumn("Enabled");
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Z");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.hasZ());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("N");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.hasN());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("H");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.hasH());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("C");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.hasC());
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IME");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.IME);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("IME Scheduled");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.IME_scheduled);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("LP Mode");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", cpu.lowPowerMode);
            ImGui::EndTable();
        }
        if (ImGui::Button("Set A to 1"))
            cpu.A(cpu.A() + 1);
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *, SDL_AppResult)
{
    /* SDL will clean up the window/renderer for us. */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
}
