#pragma once

#include <string_view>
#include "Core/Core.hpp"

#include "sdl/SDL.h"
#include "sdl/SDL_events.h"
#include "sdl/SDL_video.h"

class App {
protected:
    SDL_Window* window_ = nullptr;
    void* gl_context_ = nullptr;
    SDL_Surface* surface_ = nullptr;

    std::string_view title_;
    u32 width_;
    u32 height_;

public:
    App(std::string_view title, u32 width, u32 height) noexcept
        : title_(title)
        , width_(width)
        , height_(height)
    {}
    
    App(const App&) = delete;
    App(App&&) = delete;

    App& operator=(const App&) = delete;
    App& operator=(App&&) = delete;

    virtual ~App() noexcept;

    virtual i32 init() noexcept;
    void run() noexcept;
    virtual void render() noexcept {}
    virtual void update(f32 delta) noexcept {}
    virtual void dispatch_event(SDL_Event event) noexcept {}
};