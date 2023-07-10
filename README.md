# MiniRHI
MiniRHI is a simple OpenGL wrapper written on C++20.

# Hello Triangle example
```cpp
#include "MiniRHI/MiniRHI.hpp"

struct Vertex {
    std::array<f32, 3> position;
    std::array<f32, 3> color;

    static constexpr auto get_attrs() noexcept {
        return minirhi::VtxAttrArr<
            minirhi::VtxAttr<minirhi::format::RGB32Float_t>,
            minirhi::VtxAttr<minirhi::format::RGB32Float_t>
        >{};
    }
};
using Attrs = decltype(Vertex::get_attrs());

static constexpr std::string_view kVS = R"str(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vert_color;

void main() {
    gl_Position = vec4(position, 1.0);
    vert_color = color;
}
)str";

static constexpr std::string_view kFS = R"str(
#version 330 core

in vec3 vert_color;
out  vec4 frag_color;

void main() {
    frag_color = vec4(vert_color, 1.0);
}
)str";

static constexpr std::size_t kScreenWidth = 1280;
static constexpr std::size_t kScreenHeight = 720;

...
minirhi::init();

auto pipeline = minirhi::PipelineState<Attrs>{}
        .set_topology(minirhi::PrimitiveTopologyType::eTriangle)
        .set_vertex_shader(minirhi::ShaderCompiler::compile_from_code<minirhi::VtxShaderHandle>(kVS))
        .set_fragment_shader(minirhi::ShaderCompiler::compile_from_code<minirhi::FragShaderHandle>(kFS))
        .build();

static constexpr std::array vertices = {
        Vertex { {-1.f, -1.f, 0.f}, {1.f, 0.f, 0.f} },
        Vertex { {1.f, -1.f, 0.f}, {0.f, 1.f, 0.f} },
        Vertex { {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} },
};

auto vb = minirhi::make_vertex_buffer_rc(std::span<const Vertex>(vertices.begin(), vertices.end()));

minirhi::RenderCommands cmd;
minirhi::Viewport vp{ kScreenWidth, kScreenHeight };

auto draw_params = minirhi::make_draw_params(vp, pipeline, vb);

// game loop start
cmd.clear_color_buffer(1.0, 0.0, 0.0, 0.0);
cmd.draw(draw_params, vertices.size(), 0);
// swapping images
// game loop end
```
