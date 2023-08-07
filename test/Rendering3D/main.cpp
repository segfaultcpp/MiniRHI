#include "Core/Core.hpp"

#include "MiniRHI/Buffer.hpp"
#include "MiniRHI/Format.hpp"
#include "MiniRHI/MiniRHI.hpp"

#include "MiniRHI/PipelineState.hpp"
#include "MiniRHI/CmdCtx.hpp"
#include "MiniRHI/Texture.hpp"
#include "MiniRHI/TypeInference.hpp"

#include "sdl/SDL_error.h"
#include "sdl/SDL_events.h"
#include "sdl/SDL_keycode.h"
#include "sdl/SDL_mouse.h"
#include "sdl/SDL_video.h"
#include "sdl/SDL.h"

#include "stb/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/mat4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <glm/vec2.hpp>

#include <App.hpp>
#include <Camera.hpp>

#include <string_view>
#include <chrono>
#include <array>
#include <memory>
#include <iostream>

static constexpr std::size_t kScreenWidth = 1280;
static constexpr std::size_t kScreenHeight = 720;

struct Vertex {
    std::array<f32, 3> position;
    std::array<f32, 2> tex_coord;
};

inline constexpr static auto deleter = [](stbi_uc* data) noexcept {
    stbi_image_free(data);
};

using Attrs = minirhi::MakeVertexAttributes<Vertex>;
using Image = std::unique_ptr<stbi_uc, decltype(deleter)>;

class Rendering3D : public App {
    static constexpr std::array kObjPositions = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f),
        glm::vec3(0.f, 3.f, -3.f)
    };

    static constexpr FixedString kVS = R"str(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coord;

out vec2 vert_tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    vert_tex_coord = vec2(tex_coord.x, 1.0 - tex_coord.y);
}
)str";

    static constexpr FixedString kFS = R"str(
#version 330 core
in vec2 vert_tex_coord;

out vec4 color;
uniform sampler2D tex;

void main()
{
    color = texture(tex, vert_tex_coord);
    // color = vec4(0.84, 0.42, 0.1, 1.0);
}
)str";

    using Pipeline = decltype(minirhi::generate_graphics_pipeline_from_shaders<kVS, kFS>(minirhi::PrimitiveTopologyType::eTriangle));

    static constexpr std::array kVertices = {
        Vertex{{-0.5f, -0.5f, -0.5f},  {0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {1.0f, 0.0f}},
        Vertex{{0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
        Vertex{{0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
        Vertex{{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f}},

        Vertex{{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f}},
        Vertex{{0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
        Vertex{{0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},
        Vertex{{0.5f,  0.5f,  0.5f},  {1.0f, 1.0f}},
        Vertex{{-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f}},

        Vertex{{-0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f}},
        Vertex{{-0.5f,  0.5f, -0.5f}, { 1.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f}},
        Vertex{{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f}},
        Vertex{{-0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f}},

        Vertex{{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
        Vertex{{0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {0.0f, 1.0f}},
        Vertex{{0.5f, -0.5f,  0.5f},  {0.0f, 0.0f}},
        Vertex{{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},

        Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f}},
        Vertex{{0.5f, -0.5f, -0.5f},  {1.0f, 1.0f}},
        Vertex{{0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
        Vertex{{0.5f, -0.5f,  0.5f},  {1.0f, 0.0f}},
        Vertex{{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f}},
        Vertex{{-0.5f, -0.5f, -0.5f}, { 0.0f, 1.0f}},

        Vertex{{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f}},
        Vertex{{0.5f,  0.5f, -0.5f},  {1.0f, 1.0f}},
        Vertex{{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
        Vertex{{0.5f,  0.5f,  0.5f},  {1.0f, 0.0f}},
        Vertex{{-0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f}},
        Vertex{{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f}}
    };

    inline static const glm::mat4 proj_mat_ = glm::perspective(45.0f, GLfloat(kScreenWidth) / GLfloat(kScreenHeight), 0.1f, 100.0f);;
    glm::mat4 view_mat_ = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.0f, 0.0f, -3.0f));
    Camera camera_;
    glm::mat4 camera_rot_ = glm::identity<glm::mat4>();
    glm::vec2 cur_mouse = glm::vec2(kScreenWidth / 2.f, kScreenHeight / 2.f);
    glm::vec2 last_mouse = glm::vec2(kScreenWidth / 2.f, kScreenHeight / 2.f);

    inline static std::array<bool, 322> keys_{};

    Pipeline pipeline_;
    minirhi::VertexBufferRC<Vertex> vb_;
    minirhi::TextureRC texture_;
    Image texture_res_;

public:
    explicit Rendering3D() noexcept 
        : App("Rendering3D", kScreenWidth, kScreenHeight)
        , camera_(glm::vec3(0.f, 0.f, 3.f))
    {}

    ~Rendering3D() noexcept override = default;

    i32 init() noexcept override {
        i32 code = App::init();
        assert(code == 0);

        SDL_SetWindowGrab(window_, SDL_TRUE);
        // SDL_ShowCursor(SDL_DISABLE);
        SDL_SetRelativeMouseMode(SDL_TRUE);

        vb_.reset(std::span<const Vertex>(kVertices.begin(), kVertices.end()));
        pipeline_ = minirhi::generate_graphics_pipeline_from_shaders<kVS, kFS>(
            minirhi::PrimitiveTopologyType::eTriangle, 
            minirhi::DepthStencilDesc {
                .enable_depth = true,
            }
        );

        i32 x = 0;
        i32 y = 0;
        i32 n = 0;

        auto data = make_non_null(stbi_load("resources/images/logo.png", &x, &y, &n, 0), 
            [] {
                std::cout << std::format("Failed to load image file! Reason: {}\n", stbi_failure_reason());
                std::abort();
            }
        );

        texture_res_.reset(data.ptr);

        auto texture_format = [&n] {
            switch (n) {
            case 3: return minirhi::Format::eRGB8_UInt;
            case 4: return minirhi::Format::eRGBA8_UInt;
            default: return minirhi::Format::eUnknown;
            }
        }();
        minirhi::SamplerDesc sampler{};
        sampler.min_filter = minirhi::TextureFilter::eLinear_MipMapLinear;
        sampler.mag_filter = minirhi::TextureFilter::eLinear;

        texture_ = minirhi::make_texture_2d_rc(minirhi::SamplerDesc{}, x, y, texture_format, texture_res_.get(), true);

        return 0;
    }

    void movement_update(f32 delta) noexcept {
        constexpr f32 horizontal_scale = 0.05f;
        constexpr f32 vertical_scale = 0.05f;
        
        f32 camera_speed = delta;
        const auto right_vec = camera_.calc_right_vector();
        const auto forward_vec = camera_.calc_forward_vector();

        if (keys_[SDLK_w]) {
            camera_.position += forward_vec * camera_speed * horizontal_scale;
        }
        if (keys_[SDLK_s]) {
            camera_.position -= forward_vec * camera_speed * horizontal_scale;
        }
        if (keys_[SDLK_d]) {
            camera_.position += right_vec * camera_speed * horizontal_scale;
        }
        if (keys_[SDLK_a]) {
            camera_.position -= right_vec * camera_speed * horizontal_scale;
        }
        if (keys_[SDLK_e]) {
            camera_.position += camera_.up * camera_speed * vertical_scale;
        }
        if (keys_[SDLK_q]) {
            camera_.position -= camera_.up * camera_speed * vertical_scale;
        }
        if (keys_[SDLK_ESCAPE]) {
            SDL_ShowCursor(SDL_ENABLE);
            SDL_SetWindowGrab(window_, SDL_FALSE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }

    void camera_update(f32 delta) noexcept {
        constexpr f32 sensitivity = 0.005f;
        glm::vec2 mouse_off = (cur_mouse - last_mouse) * sensitivity;
        last_mouse = cur_mouse;

        camera_.rotation.head += mouse_off.x;
        camera_.rotation.pitch += mouse_off.y;
    }

    void update(f32 delta) noexcept override {
        // delta = 16.6f;
        movement_update(delta);
        camera_update(delta);
    }

    void dispatch_event(SDL_Event event) noexcept override {
        switch (event.type) {
        case SDL_KEYDOWN:
            if (std::size_t(event.key.keysym.sym) < keys_.size()) {
                keys_[std::size_t(event.key.keysym.sym)] = true;
            }
            break;
        case SDL_KEYUP:
            if (std::size_t(event.key.keysym.sym) < keys_.size()) {
                keys_[std::size_t(event.key.keysym.sym)] = false;
            }
            break;
        case SDL_MOUSEMOTION:
            i32 x = 0;
            i32 y = 0;
            auto _ = SDL_GetMouseState(&x, &y);

            cur_mouse.x = f32(x);
            cur_mouse.y = f32(y);
            break;
        }
    }

    void render() noexcept override {
        static constexpr auto kVP = minirhi::Viewport(kScreenWidth, kScreenHeight);
        auto bindings = minirhi::make_bindings(
            minirhi::Mat4Slot<"projection">(proj_mat_),
            minirhi::Mat4Slot<"model">(),
            minirhi::Mat4Slot<"view">(camera_.look_at()),
            minirhi::Texture2DSlot<"tex">(texture_)
        );
        auto draw_params = minirhi::make_draw_params(kVP, pipeline_);
        auto draw_ctx = minirhi::CmdCtx::start_draw_context(draw_params);

        minirhi::CmdCtx::clear_color_buffer(0.f, 0.749, 1.f, 1.f);
        minirhi::CmdCtx::clear_depth_buffer();
        std::size_t i = 0;
        for (auto pos : kObjPositions) {
            glm::mat4 model = glm::translate(glm::identity<glm::mat4>(), pos);
            
            GLfloat angle = 20.0f * i++;
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.3f, 0.5f));

            bindings.get_mat4_slot<"model">().value = model;
            draw_ctx.set_bindings(bindings);
            draw_ctx.draw(vb_, kVertices.size(), 0);
        }
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    Rendering3D app;
    app.init();
    app.run();
    return 0;
}