#include <iostream>

#include "Core/Core.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/MiniRHI.hpp"

#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RenderCommands.hpp"
#include "MiniRHI/Shader.hpp"
#include "sdl/SDL_error.h"
#include "sdl/SDL_keycode.h"
#include "sdl/SDL_video.h"
#include <sdl/SDL.h>

static constexpr std::size_t kScreenWidth = 1280;
static constexpr std::size_t kScreenHeight = 720;

int main(int argc, char* args[]) {
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
        std::array<f32, 3> position;

        static constexpr auto get_attrs() noexcept {
            return minirhi::VtxAttrArr<
                minirhi::VtxAttr<minirhi::format::RGB32Float_t>
            >{};
        }
    };
    
    using Attrs = decltype(Vertex::get_attrs());

    static constexpr std::string_view kVS = R"str(
#version 330 core
layout (location = 0) in vec3 position;

void main() {
    gl_Position = vec4(position, 1.0);
})str";

    static constexpr std::string_view kFS = R"str(
#version 330 core
out  vec4 frag_color;

void main() {
    frag_color = vec4(0.0, 1.0, 1.0, 1.0);
}
)str";

    auto pipeline = minirhi::PipelineState<Attrs>{}
        .set_topology(minirhi::PrimitiveTopologyType::eTriangle)
        .set_vertex_shader(minirhi::ShaderCompiler::compile_from_code<minirhi::VtxShaderHandle>(kVS))
        .set_fragment_shader(minirhi::ShaderCompiler::compile_from_code<minirhi::FragShaderHandle>(kFS))
        .build();

    static constexpr std::array vertices = {
        Vertex { {-1.f, -1.f, 0.f} },
        Vertex { {1.f, -1.f, 0.f} },
        Vertex { {0.f, 1.f, 0.f} },
    }; 
    minirhi::BufferDesc desc{ minirhi::BufferType::eVertex, std::span(vertices) };
    auto vb = minirhi::make_buffer_rc(desc);
    // std::cout << "Hello RenderDoc! Passed vb initialization.\n";
    
    minirhi::RenderCommands cmd;
    minirhi::Viewport vp{ kScreenWidth, kScreenHeight };

    auto draw_params = minirhi::make_draw_params(vp, pipeline, vb);

    while(!quit) { 
        while(SDL_PollEvent( &e )) { 
            if(e.type == SDL_QUIT) {
                quit = true;
            }
        }
        cmd.clear_color_buffer(1.0, 0.0, 0.0, 0.0);
        cmd.draw(draw_params, vertices.size(), 0);
        SDL_GL_SwapWindow(window.ptr);
    }

    SDL_GL_DeleteContext(gl_context.ptr);
    SDL_DestroyWindow(window.ptr);
    SDL_Quit();
    return 0;
}