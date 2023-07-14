#include <iostream>

#include "Core/Core.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/MiniRHI.hpp"

#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/RenderCommands.hpp"
#include "MiniRHI/Shader.hpp"
#include "MiniRHI/Texture.hpp"
#include "MiniRHI/TypeInference.hpp"

#include "sdl/SDL_error.h"
#include "sdl/SDL_keycode.h"
#include "sdl/SDL_video.h"
#include <sdl/SDL.h>
#include <stb/stb_image.h>

#include <App.hpp>

#include <format>
#include <memory>

struct Vertex {
    std::array<f32, 2> position;
    std::array<f32, 3> color;
};
using Attrs = minirhi::MakeVertexAttributes<Vertex>;

static constexpr std::size_t kScreenWidth = 1280;
static constexpr std::size_t kScreenHeight = 720;

class HelloTriangle final : public App {
    static constexpr FixedString kVS = R"str(
#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;

out vec3 vert_color;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vert_color = color;
}
)str";

    static constexpr FixedString kFS = R"str(
#version 330 core

in vec3 vert_color;

out  vec4 frag_color;

void main() {
    frag_color = vec4(vert_color, 1.0);
}
)str";

    static constexpr std::array vertices = {
        Vertex { {-1.f, -1.f}, {1.f, 1.f, 0.f} },
        Vertex { {1.f, -1.f}, {0.f, 1.f, 1.f} },
        Vertex { {-0.f, 1.f}, {1.f, 0.f, 1.f} }
    };

    using Pipeline = decltype(
        minirhi::generate_pipeline_from_shaders<kVS, kFS>(
            minirhi::PrimitiveTopologyType::eCount, 
            minirhi::RasterizerStateDesc{}
        )
    );

    Pipeline pipeline_;
    minirhi::VertexBufferRC<Vertex> vb_;
    minirhi::RenderCommands cmds_;
public:
    HelloTriangle() noexcept = default;
    ~HelloTriangle() noexcept override = default;

    i32 init(std::string_view title, u32 width, u32 height) noexcept override {
        i32 code = App::init(title, width, height);
        assert(code == 0);

        pipeline_ = minirhi::generate_pipeline_from_shaders<kVS, kFS>(
            minirhi::PrimitiveTopologyType::eTriangle, 
            minirhi::RasterizerStateDesc{}
        );

        vb_.reset(std::span<const Vertex>(vertices.begin(), vertices.end()));
        cmds_ = minirhi::make_render_commands();
        return 0;
    }

    void render() noexcept override {
        minirhi::Viewport vp{ kScreenWidth, kScreenHeight };
        auto draw_params = minirhi::make_draw_params(vp, pipeline_, vb_);
        
        cmds_.clear_color_buffer(1.0, 0.0, 0.0, 0.0);
        cmds_.draw(draw_params, vertices.size(), 0);
    }
};

int SDL_main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    HelloTriangle app;
    app.init("HelloTriangle", kScreenWidth, kScreenHeight);
    app.run();
    return 0;
}