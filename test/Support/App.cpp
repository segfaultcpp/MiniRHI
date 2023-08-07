#include "App.hpp"

#include "sdl/SDL_error.h"
#include "sdl/SDL_events.h"
#include "sdl/SDL_keycode.h"
#include "sdl/SDL_mouse.h"
#include "sdl/SDL_timer.h"
#include "sdl/SDL_video.h"

#include "MiniRHI/MiniRHI.hpp"
#include "Core/Core.hpp"

#include <iostream>
#include <format>
#include <chrono>

i32 App::init() noexcept {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
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
        SDL_CreateWindow(title_.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, i32(width_), i32(height_), SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN), 
        sdl_error_handler
    );
    window_ = window.ptr;
    
    auto gl_context = make_non_null(SDL_GL_CreateContext(window.ptr), sdl_error_handler);
    gl_context_ = gl_context.ptr;
    surface_ = SDL_GetWindowSurface(window.ptr);

    minirhi::init();
    SDL_GL_SetSwapInterval(1);
    SDL_UpdateWindowSurface(window.ptr);
    return 0;
}

void App::run() noexcept {
    using namespace std::chrono;

    bool quit = false;
    SDL_Event e;
    f32 delta = 16.6f;
    while(!quit) {
        u64 start = SDL_GetTicks64();
        while(SDL_PollEvent( &e ) != 0) { 
            if(e.type == SDL_QUIT) {
                quit = true;
            } else {
                this->dispatch_event(e);
            }
        }
        this->update(delta);
        this->render();

        SDL_GL_SwapWindow(window_);
        delta = f32(SDL_GetTicks64() - start);
    }
}

App::~App() noexcept {
    SDL_GL_DeleteContext(gl_context_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}
