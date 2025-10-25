// gui_newton.cpp
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <fstream> 

#include "render_newton.h"
#include "lodepng.h"

// ---------- SDL2 + OpenGL + ImGui ----------
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

// Save helpers (reuse your functions)
static void writePNG(const FrameBuff &fb, const std::string &filename) {
    const size_t N = fb.width * fb.height;
    std::vector<unsigned char> interleaved(3 * N);
    for (size_t i = 0; i < N; ++i) {
        interleaved[3*i + 0] = fb.red[i];
        interleaved[3*i + 1] = fb.green[i];
        interleaved[3*i + 2] = fb.blue[i];
    }
    unsigned err = lodepng::encode(filename, interleaved, fb.width, fb.height, LCT_RGB);
    if (err) throw std::runtime_error("PNG encode error " + std::to_string(err));
}

static void writePPM(const FrameBuff &fb, const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open " + filename);
    out << "P6\n" << fb.width << " " << fb.height << "\n255\n";
    const size_t N = fb.width * fb.height;
    for (size_t i = 0; i < N; ++i) {
        out.put(static_cast<char>(fb.red[i]));
        out.put(static_cast<char>(fb.green[i]));
        out.put(static_cast<char>(fb.blue[i]));
    }
}

// Upload our CPU framebuffer into an OpenGL texture
static GLuint uploadTextureRGB(const FrameBuff& fb, GLuint existing = 0) {
    if (existing == 0) glGenTextures(1, &existing);
    glBindTexture(GL_TEXTURE_2D, existing);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const size_t N = fb.width * fb.height;
    std::vector<unsigned char> interleaved(3 * N);
    for (size_t i = 0; i < N; ++i) {
        interleaved[3*i+0] = fb.red[i];
        interleaved[3*i+1] = fb.green[i];
        interleaved[3*i+2] = fb.blue[i];
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)fb.width, (GLsizei)fb.height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, interleaved.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return existing;
}

int main(int, char**)
{
    // ---- Init SDL + GL ----
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL Error: " << SDL_GetError() << "\n";
        return 1;
    }
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_Window* window = SDL_CreateWindow("Newton Fractal GUI",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1200, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    // ---- ImGui setup ----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ---- Parameters & framebuffer ----
    int            power    = 3;
    int            width_i  = 500;  // sliders use int; cast to size_t later
    int            height_i = 500;
    int            max_iter_i = 25;
    double         min_step2 = 1e-6;
    bool           request_render = true;   // render once at start
    bool           lock_aspect = true;

    FrameBuff fb;
    GLuint tex = 0;
    std::string last_error;

    auto do_render = [&](){
        last_error.clear();
        if (width_i < 1)  width_i  = 1;
        if (height_i < 1) height_i = 1;
        if (power < 1)    power    = 1;
        if (max_iter_i < 1) max_iter_i = 1;
        if (min_step2 <= 0.0) min_step2 = 1e-12;

        std::string err;
        bool ok = render_newton_frame(
            power,
            (size_t)width_i,
            (size_t)height_i,
            (unsigned short)max_iter_i,
            min_step2,
            fb,
            &err
        );
        if (!ok) {
            last_error = err.empty() ? "Unknown compute error." : err;
        } else {
            tex = uploadTextureRGB(fb, tex);
        }
    };

    // ---- Main loop ----
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) done = true;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) done = true;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Control panel
        ImGui::Begin("Controls");

        if (ImGui::SliderInt("power", &power, 1, 64)) request_render = true;
        if (ImGui::SliderInt("max_iter", &max_iter_i, 1, 1000)) request_render = true;
        if (ImGui::Checkbox("lock aspect (1:1)", &lock_aspect)) {}
        if (ImGui::SliderInt("width", &width_i, 64, 10000)) {
            if (lock_aspect) height_i = width_i;
        }
        if (ImGui::SliderInt("height", &height_i, 64, 10000)) {
            if (lock_aspect) width_i = height_i;
        }
        if (ImGui::InputDouble("min_step2", &min_step2, 0, 0, "%.1e")) request_render = true;

        if (ImGui::Button("Render")) request_render = true;
        ImGui::SameLine();
        if (ImGui::Button("Save PNG")) {
            try {
                // timestamp name
                std::time_t t = std::time(nullptr);
                char buf[64]; std::strftime(buf, sizeof(buf), "NEWTON_%Y%m%d_%H%M%S.png", std::localtime(&t));
                writePNG(fb, buf);
            } catch (const std::exception& e) {
                last_error = std::string("Save PNG failed: ") + e.what();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save PPM")) {
            try {
                std::time_t t = std::time(nullptr);
                char buf[64]; std::strftime(buf, sizeof(buf), "NEWTON_%Y%m%d_%H%M%S.ppm", std::localtime(&t));
                writePPM(fb, buf);
            } catch (const std::exception& e) {
                last_error = std::string("Save PPM failed: ") + e.what();
            }
        }

        if (!last_error.empty()) {
            ImGui::TextColored(ImVec4(1,0.5f,0.5f,1), "Error: %s", last_error.c_str());
        }

        ImGui::End();

        // Image panel
        ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse);
        if (request_render) {
            do_render();
            request_render = false;
        }
        if (tex != 0) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            // keep aspect of framebuffer
            float fb_aspect = (fb.height > 0) ? (float)fb.width / (float)fb.height : 1.0f;
            float draw_w = avail.x;
            float draw_h = draw_w / fb_aspect;
            if (draw_h > avail.y) { draw_h = avail.y; draw_w = draw_h * fb_aspect; }
            ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2(draw_w, draw_h));
        } else {
            ImGui::TextUnformatted("No image yet. Click Render.");
        }
        ImGui::End();

        // Render UI
        ImGui::Render();
        glViewport(0,0,(int)io.DisplaySize.x,(int)io.DisplaySize.y);
        glClearColor(0.08f,0.08f,0.1f,1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    if (tex) glDeleteTextures(1, &tex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
