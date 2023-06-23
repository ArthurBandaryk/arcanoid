// ImGui SDL2 binding with OpenGL3
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture
// identifier. Read the FAQ about ImTextureID in imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows,
// inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no
// standard header to access modern OpenGL functions easily. Alternatives are
// GLEW, Glad, etc.)

// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this.
// If you use this binding you'll need to call 4 functions:
// ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and
// ImGui_ImplXXXX_Shutdown().
// If you are new to ImGui, see examples/README.txt and documentation at the top
// of imgui.cpp.
// https://github.com/ocornut/imgui

#include "imgui_impl_opengl.hxx"

#include "opengl-debug.hxx"

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_syswm.h>
#include <SDL3/SDL_video.h>

#include <glad/glad.h>

#include <chrono>
#include <iostream>

// Data
static std::chrono::time_point<std::chrono::steady_clock> g_Time
    = std::chrono::steady_clock::now();
static bool g_MousePressed[3] = { false, false, false };
static float g_MouseWheel = 0.0f;
static GLuint g_FontTexture = 0;
static int g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int g_AttribLocationPosition = 0, g_AttribLocationUV = 0,
           g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;

// This is the main rendering function that you have to implement and provide to
// ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// Note that this implementation is little overcomplicated because we are
// saving/setting up/restoring every OpenGL state explicitly, in order to be
// able to run within any OpenGL engine that doesn't do so.
// If text or lines are blurry when integrating ImGui in your engine: in your
// Render function, try translating your projection matrix by (0.5f,0.5f) or
// (0.375f,0.375f)
void ImGui_ImplSdlGL3_RenderDrawLists(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays
    // (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    arci::opengl_check();
    // Backup GL state
    GLenum last_active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    arci::opengl_check();
    glActiveTexture(GL_TEXTURE0);
    arci::opengl_check();
    GLint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    arci::opengl_check();
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    arci::opengl_check();
    GLint last_sampler;
    glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
    arci::opengl_check();
    GLint last_array_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    arci::opengl_check();
    GLint last_element_array_buffer;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    arci::opengl_check();
    GLint last_vertex_array;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    arci::opengl_check();
    // GLint last_polygon_mode[2]; open gl 4?
    // glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    arci::opengl_check();
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    arci::opengl_check();
    GLint last_scissor_box[4];
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    arci::opengl_check();
    GLenum last_blend_src_rgb;
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    arci::opengl_check();
    GLenum last_blend_dst_rgb;
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    arci::opengl_check();
    GLenum last_blend_src_alpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    arci::opengl_check();
    GLenum last_blend_dst_alpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    arci::opengl_check();
    GLenum last_blend_equation_rgb;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    arci::opengl_check();
    GLenum last_blend_equation_alpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    arci::opengl_check();
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    arci::opengl_check();
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    arci::opengl_check();
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    arci::opengl_check();
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
    arci::opengl_check();
    // Setup render state: alpha-blending enabled, no face culling, no depth
    // testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    arci::opengl_check();
    glBlendEquation(GL_FUNC_ADD);
    arci::opengl_check();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    arci::opengl_check();
    glDisable(GL_CULL_FACE);
    arci::opengl_check();
    glDisable(GL_DEPTH_TEST);
    arci::opengl_check();
    glEnable(GL_SCISSOR_TEST);
    arci::opengl_check();
    // no in opengl es 2.0
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    arci::opengl_check();

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float ortho_projection[4][4] = {
        { 2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f, 1.0f },
    };
    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(
        g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

    arci::opengl_check();

    glBindVertexArray(g_VaoHandle);
    arci::opengl_check();

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        arci::opengl_check();
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                     (const GLvoid*)cmd_list->VtxBuffer.Data,
                     GL_STREAM_DRAW);
        arci::opengl_check();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        arci::opengl_check();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                     (const GLvoid*)cmd_list->IdxBuffer.Data,
                     GL_STREAM_DRAW);
        arci::opengl_check();

        glEnableVertexAttribArray(g_AttribLocationPosition);
        arci::opengl_check();
        glEnableVertexAttribArray(g_AttribLocationUV);
        arci::opengl_check();
        glEnableVertexAttribArray(g_AttribLocationColor);
        arci::opengl_check();

        glVertexAttribPointer(g_AttribLocationPosition,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(ImDrawVert),
                              (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
        arci::opengl_check();
        glVertexAttribPointer(g_AttribLocationUV,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(ImDrawVert),
                              (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
        arci::opengl_check();
        glVertexAttribPointer(g_AttribLocationColor,
                              4,
                              GL_UNSIGNED_BYTE,
                              GL_TRUE,
                              sizeof(ImDrawVert),
                              (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
        arci::opengl_check();

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                arci::opengl_check();
                glScissor((int)pcmd->ClipRect.x,
                          (int)(fb_height - pcmd->ClipRect.w),
                          (int)(pcmd->ClipRect.z - pcmd->ClipRect.x),
                          (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                arci::opengl_check();
                glDrawElements(GL_TRIANGLES,
                               (GLsizei)pcmd->ElemCount,
                               sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                                      : GL_UNSIGNED_INT,
                               idx_buffer_offset);
                arci::opengl_check();
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state
    glUseProgram(last_program);
    arci::opengl_check();
    glBindTexture(GL_TEXTURE_2D, last_texture);
    arci::opengl_check();
    // glBindSampler(0, last_sampler);
    glActiveTexture(last_active_texture);
    arci::opengl_check();
    // glBindVertexArray(last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb,
                        last_blend_dst_rgb,
                        last_blend_src_alpha,
                        last_blend_dst_alpha);
    arci::opengl_check();
    if (last_enable_blend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    if (last_enable_cull_face)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    if (last_enable_depth_test)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);

    arci::opengl_check();
    // no in opengl es 2.0
    // glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
    arci::opengl_check();
    glViewport(last_viewport[0],
               last_viewport[1],
               (GLsizei)last_viewport[2],
               (GLsizei)last_viewport[3]);
    arci::opengl_check();
    glScissor(last_scissor_box[0],
              last_scissor_box[1],
              (GLsizei)last_scissor_box[2],
              (GLsizei)last_scissor_box[3]);
    arci::opengl_check();
}

static const char* ImGui_ImplSdlGL3_GetClipboardText(void*)
{
    return SDL_GetClipboardText();
}

static void ImGui_ImplSdlGL3_SetClipboardText(void*, const char* text)
{
    SDL_SetClipboardText(text);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if
// dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your
// main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to
// your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from
// your application based on those two flags.
bool ImGui_ImplSdlGL3_ProcessEvent(SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type)
    {
    case SDL_EVENT_MOUSE_WHEEL: {
        if (event->wheel.y > 0)
            g_MouseWheel = 1;
        if (event->wheel.y < 0)
            g_MouseWheel = -1;
        return true;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        if (event->button.button == SDL_BUTTON_LEFT)
            g_MousePressed[0] = true;
        if (event->button.button == SDL_BUTTON_RIGHT)
            g_MousePressed[1] = true;
        if (event->button.button == SDL_BUTTON_MIDDLE)
            g_MousePressed[2] = true;
        return true;
    }
    case SDL_EVENT_TEXT_INPUT: {
        io.AddInputCharactersUTF8(event->text.text);
        return true;
    }
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
        io.KeysDown[key] = (event->type == SDL_EVENT_KEY_DOWN);
        io.KeyShift = ((SDL_GetModState() & SDL_KMOD_SHIFT) != 0);
        io.KeyCtrl = ((SDL_GetModState() & SDL_KMOD_CTRL) != 0);
        io.KeyAlt = ((SDL_GetModState() & SDL_KMOD_ALT) != 0);
        io.KeySuper = ((SDL_GetModState() & SDL_KMOD_GUI) != 0);
        return true;
    }
    }
    return false;
}

void ImGui_ImplSdlGL3_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(
        &pixels, &width, &height); // Load as RGBA 32-bits for OpenGL3 demo
                                   // because it is more likely to be compatible
                                   // with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    arci::opengl_check();
    glGenTextures(1, &g_FontTexture);
    arci::opengl_check();
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    arci::opengl_check();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    arci::opengl_check();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    arci::opengl_check();
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    arci::opengl_check();
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 width,
                 height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 pixels);
    arci::opengl_check();

    // Store our identifier
    io.Fonts->TexID = (void*)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    arci::opengl_check();
}

bool ImGui_ImplSdlGL3_CreateDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    arci::opengl_check();
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    arci::opengl_check();
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    arci::opengl_check();

    const GLchar* vertex_shader =
        //"#version 150\n"
        "#if defined(GL_ES)\n"
        "precision highp float;\n"
        "#endif //GL_ES\n"
        "uniform mat4 ProjMtx;\n"
        "attribute vec2 Position;\n"
        "attribute vec2 UV;\n"
        "attribute vec4 Color;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        //"#version 150\n"
        "#if defined(GL_ES)\n"
        "precision highp float;\n"
        "#endif //GL_ES\n"
        "uniform sampler2D Texture;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        //"out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	gl_FragColor = Frag_Color * texture2D( Texture, Frag_UV);\n"
        "}\n";

    g_ShaderHandle = glCreateProgram();
    arci::opengl_check();
    g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
    arci::opengl_check();
    g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    arci::opengl_check();
    glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    arci::opengl_check();
    glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    arci::opengl_check();
    glCompileShader(g_VertHandle);
    arci::opengl_check();
    glCompileShader(g_FragHandle);
    arci::opengl_check();
    glAttachShader(g_ShaderHandle, g_VertHandle);
    arci::opengl_check();
    glAttachShader(g_ShaderHandle, g_FragHandle);
    arci::opengl_check();
    glLinkProgram(g_ShaderHandle);
    arci::opengl_check();

    g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

    arci::opengl_check();

    glGenBuffers(1, &g_VboHandle);
    arci::opengl_check();
    glGenBuffers(1, &g_ElementsHandle);
    arci::opengl_check();

    glGenVertexArrays(1, &g_VaoHandle);
    arci::opengl_check();
    glBindVertexArray(g_VaoHandle);
    arci::opengl_check();
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    arci::opengl_check();

    glEnableVertexAttribArray(g_AttribLocationPosition);
    arci::opengl_check();
    glEnableVertexAttribArray(g_AttribLocationUV);
    arci::opengl_check();
    glEnableVertexAttribArray(g_AttribLocationColor);
    arci::opengl_check();

    glVertexAttribPointer(g_AttribLocationPosition,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    arci::opengl_check();
    glVertexAttribPointer(g_AttribLocationUV,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    arci::opengl_check();
    glVertexAttribPointer(g_AttribLocationColor,
                          4,
                          GL_UNSIGNED_BYTE,
                          GL_TRUE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
    arci::opengl_check();

    ImGui_ImplSdlGL3_CreateFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    arci::opengl_check();
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    arci::opengl_check();
    // glBindVertexArray(last_vertex_array);

    return true;
}

void ImGui_ImplSdlGL3_InvalidateDeviceObjects()
{
    if (g_VaoHandle)
    {
        //   glDeleteVertexArrays(1, &g_VaoHandle);
    }
    if (g_VboHandle)
        glDeleteBuffers(1, &g_VboHandle);
    if (g_ElementsHandle)
        glDeleteBuffers(1, &g_ElementsHandle);
    g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

    if (g_ShaderHandle && g_VertHandle)
        glDetachShader(g_ShaderHandle, g_VertHandle);
    if (g_VertHandle)
        glDeleteShader(g_VertHandle);
    g_VertHandle = 0;

    if (g_ShaderHandle && g_FragHandle)
        glDetachShader(g_ShaderHandle, g_FragHandle);
    if (g_FragHandle)
        glDeleteShader(g_FragHandle);
    g_FragHandle = 0;

    if (g_ShaderHandle)
        glDeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    if (g_FontTexture)
    {
        glDeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

bool ImGui_ImplSdlGL3_Init(SDL_Window* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    io.Fonts->AddFontFromFileTTF("res/Roboto-Medium.ttf", 50.f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    io.BackendPlatformName = "sdl_engine";

    io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB; // Keyboard mapping. ImGui will use
                                                // those indices to peek into the
                                                // io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;

    io.SetClipboardTextFn = ImGui_ImplSdlGL3_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplSdlGL3_GetClipboardText;
    io.ClipboardUserData = NULL;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_GetWindowWMInfo(window, &wmInfo, SDL_SYSWM_CURRENT_VERSION);
    io.ImeWindowHandle = wmInfo.info.win.window;
#else
    (void)window;
#endif

    return true;
}

void ImGui_ImplSdlGL3_Shutdown()
{
    ImGui_ImplSdlGL3_InvalidateDeviceObjects();
    ImGui::DestroyContext();
}

void ImGui_ImplSdlGL3_NewFrame(SDL_Window* window)
{
    if (!g_FontTexture)
        ImGui_ImplSdlGL3_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GetWindowSizeInPixels(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0,
                                        h > 0 ? ((float)display_h / h) : 0);

    std::chrono::time_point<std::chrono::steady_clock> current_time
        = std::chrono::steady_clock::now();
    std::chrono::duration<float> duration = current_time - g_Time;

    float delta_time = std::chrono::duration_cast<std::chrono::microseconds>(
                           current_time - g_Time)
                           .count();
    delta_time /= 1'000'000.f;
    io.DeltaTime = delta_time;

    if (delta_time < 0.f)
    {
        io.DeltaTime = 0.00001f;
    }

    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from
    // SDL_PollEvent())
    float mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2(mx, my);
    else
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    io.MouseDown[0] = g_MousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0; // If a mouse press event came, always pass
                                                                                           // it as "mouse held this frame", so we
                                                                                           // don't miss click-release events that are
                                                                                           // shorter than 1 frame.
    io.MouseDown[1] = g_MousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = g_MousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    if (io.MouseDrawCursor)
    {
        SDL_HideCursor();
    }
    else
    {
        SDL_ShowCursor();
    }

    // Start the frame. This call will update the io.WantCaptureMouse,
    // io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not)
    // to your application.
    ImGui::NewFrame();
}
