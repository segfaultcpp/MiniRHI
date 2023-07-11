#include <iostream>

#include "Core/Core.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/MiniRHI.hpp"

#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RenderCommands.hpp"
#include "MiniRHI/Shader.hpp"
#include "MiniRHI/TypeInference.hpp"
#include "sdl/SDL_error.h"
#include "sdl/SDL_keycode.h"
#include "sdl/SDL_video.h"
#include <sdl/SDL.h>

static constexpr std::size_t kScreenWidth = 1280;
static constexpr std::size_t kScreenHeight = 720;


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    auto sdl_error_handler = 
        [] {
            std::cerr << std::format("Couldn't create window!\nReason: {}\n", SDL_GetError());
        };
    auto window = make_non_null(
        SDL_CreateWindow("MiniRHITest", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, kScreenWidth, kScreenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN), 
        sdl_error_handler
    );
    auto gl_context = make_non_null(SDL_GL_CreateContext(window.ptr), sdl_error_handler);
    auto* screen_surface = SDL_GetWindowSurface(window.ptr);

    minirhi::init();
    SDL_GL_SetSwapInterval(1);

    SDL_UpdateWindowSurface(window.ptr);

    SDL_Event e; 
    bool quit = false; 
    
    struct Vertex {
        std::array<f32, 2> position;
        std::array<f32, 3> color;
    };

    using Attrs = minirhi::MakeVertexAttributes<Vertex>;

    static constexpr FixedString kVS = R"str(
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

out vec3 vert_color;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vert_color = color;
})str";

    static constexpr FixedString kFS = R"str(
#version 330 core

in vec3 vert_color;
out  vec4 frag_color;

void main() {
    frag_color = vec4(vert_color, 1.0);
}
)str";

    auto pipeline = minirhi::generate_pipeline_from_shaders<kVS, kFS>(
        minirhi::PrimitiveTopologyType::eTriangle, 
        minirhi::RasterizerStateDesc{}
    );

    static constexpr std::array vertices = {
        Vertex { {-1.f, -1.f}, {1.f, 0.f, 0.f} },
        Vertex { {0.f, -1.f}, {0.f, 1.f, 0.f} },
        Vertex { {-0.5f, 1.f}, {0.f, 0.f, 1.f} },

        Vertex { {0.f, 1.f}, {1.f, 1.f, 0.f} },
        Vertex { {1.f, 1.f}, {0.f, 1.f, 1.f} },
        Vertex { {0.5f, -1.f}, {1.f, 0.f, 1.f} },
    };

    static constexpr std::array indexed_vertices = {
        Vertex { {-0.5f, -0.5f}, {1.f, 0.f, 0.f} },
        Vertex { {0.5f, -0.5f}, {0.f, 1.f, 0.f} },
        Vertex { {-0.5f, 0.5f}, {0.f, 0.f, 1.f} },
        Vertex { {0.5f, 0.5f}, {1.f, 1.f, 0.f} },
    };

    static constexpr std::array indices = {
        u32(0), u32(1), u32(2), u32(2), u32(1), u32(3)
    };

    auto vb = minirhi::make_vertex_buffer_rc(std::span<const Vertex>(vertices.begin(), vertices.end()));
    auto indexed_vb = minirhi::make_vertex_buffer_rc(std::span<const Vertex>(indexed_vertices.begin(), indexed_vertices.end()));
    auto ib = minirhi::make_index_buffer_rc(std::span<const u32>(indices.begin(), indices.end()));

    minirhi::RenderCommands cmd;
    minirhi::Viewport vp{ kScreenWidth, kScreenHeight };

    auto draw_params = minirhi::make_draw_params(vp, pipeline, vb);
    auto indexed_draw_params = minirhi::make_draw_params_indexed(vp, pipeline, indexed_vb, ib);

    while(!quit) { 
        while(SDL_PollEvent( &e ) != 0) { 
            if(e.type == SDL_QUIT) {
                quit = true;
            }
        }
        cmd.clear_color_buffer(1.0, 0.0, 0.0, 0.0);
        cmd.draw_indexed(indexed_draw_params, indices.size(), 0);
        // cmd.draw(draw_params, vertices.size(), 0);
        SDL_GL_SwapWindow(window.ptr);
    }

    SDL_GL_DeleteContext(gl_context.ptr);
    SDL_DestroyWindow(window.ptr);
    SDL_Quit();
    return 0;
}